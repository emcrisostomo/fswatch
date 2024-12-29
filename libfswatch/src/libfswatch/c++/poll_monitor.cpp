/*
 * Copyright (c) 2014-2024 Enrico M. Crisostomo
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 3, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "libfswatch/gettext_defs.h"
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <utility>
#include <unordered_map>
#include <libfswatch/libfswatch_config.h>
#include "libfswatch/c/libfswatch_log.h"
#include "poll_monitor.hpp"
#include "path_utils.hpp"

#if defined HAVE_STRUCT_STAT_ST_MTIME
#  define FSW_MTIME(stat) ((stat).st_mtime)
#  define FSW_CTIME(stat) ((stat).st_ctime)
#elif defined HAVE_STRUCT_STAT_ST_MTIMESPEC
#  define FSW_MTIME(stat) (stat.st_mtimespec.tv_sec)
#  define FSW_CTIME(stat) (stat.st_ctimespec.tv_sec)
#else
#  error "Either HAVE_STRUCT_STAT_ST_MTIME or HAVE_STRUCT_STAT_ST_MTIMESPEC must be defined"
#endif

namespace fsw
{
  using std::vector;
  using std::string;
  using std::filesystem::path;

  using poll_monitor_data = struct poll_monitor::poll_monitor_data
  {
    std::unordered_map<string, poll_monitor::watched_file_info> tracked_files;
  };

  poll_monitor::poll_monitor(vector<string> paths,
                             FSW_EVENT_CALLBACK *callback,
                             void *context) :
    monitor(std::move(paths), callback, context)
  {
    previous_data.reset(new poll_monitor_data());
    new_data.reset(new poll_monitor_data());
    time(&curr_time);
  }

  poll_monitor::~poll_monitor() = default;

  bool poll_monitor::initial_scan_callback(const string& path,
                                           const struct stat& stat)
  {
    if (previous_data->tracked_files.count(path))
      return false;

    watched_file_info wfi{FSW_MTIME(stat), FSW_CTIME(stat)};
    previous_data->tracked_files[path] = wfi;

    return true;
  }

  bool poll_monitor::intermediate_scan_callback(const string& path,
                                                const struct stat& stat)
  {
    if (new_data->tracked_files.count(path)) return false;

    const auto mtime = FSW_MTIME(stat);
    const auto ctime = FSW_CTIME(stat);

    watched_file_info wfi{mtime, ctime};
    new_data->tracked_files[path] = wfi;

    if (!previous_data->tracked_files.count(path))
    {
      vector<fsw_event_flag> flags;
      flags.push_back(fsw_event_flag::Created);
      events.emplace_back(path, curr_time, flags);

      return true;
    }
    
    watched_file_info pwfi = previous_data->tracked_files[path];
    vector<fsw_event_flag> flags;

    if (mtime > pwfi.mtime)
    {
      flags.push_back(fsw_event_flag::Updated);
    } 

    if (ctime > pwfi.ctime)
    {
      flags.push_back(fsw_event_flag::AttributeModified);
    }

    if (!flags.empty())
    {
      events.emplace_back(path, curr_time, flags);
    }

    previous_data->tracked_files.erase(path);

    return true;
  }

  bool poll_monitor::add_path(const string& path,
                              const struct stat& fd_stat,
                              const path_visitor& poll_callback)
  {
    return poll_callback(path, fd_stat);
  }

  void poll_monitor::scan(const path& path, const path_visitor& fn)
  {
    try
    {
      // Check if the path is a symbolic link
      if (follow_symlinks && std::filesystem::is_symlink(std::filesystem::symlink_status(path)))
      {
        auto link_path = std::filesystem::read_symlink(path);
        scan(link_path, fn);
        return;
      }

      if (!accept_path(path)) return;

      // TODO: C++17 doesn't standardize access to ctime, so we need to keep
      // using lstat for now.
      struct stat fd_stat;
      if (!lstat_path(path, fd_stat)) return;

      if (!add_path(path, fd_stat, fn)) return;
      if (!recursive) return;
      if (!S_ISDIR(fd_stat.st_mode)) return;

      const auto entries = get_directory_entries(path);

      for (const auto& entry : entries)
      {
        scan(entry.path(), fn);
      }
    }
    catch (const std::filesystem::filesystem_error& e) 
    {
        // Handle errors, such as permission issues or non-existent paths
        FSW_ELOGF(_("Filesystem error: %s"), e.what());
    }
  }

  void poll_monitor::find_removed_files()
  {
    vector<fsw_event_flag> flags;
    flags.push_back(fsw_event_flag::Removed);

    for (const auto& [key, value] : previous_data->tracked_files)
    {
      events.emplace_back(key, curr_time, flags);
    }
  }

  void poll_monitor::swap_data_containers()
  {
    previous_data = std::move(new_data);
    new_data.reset(new poll_monitor_data());
  }

  void poll_monitor::collect_data()
  {
    path_visitor fn = std::bind(&poll_monitor::intermediate_scan_callback, 
                                this,
                                std::placeholders::_1,
                                std::placeholders::_2);


    for (const string& path : paths)
    {
      scan(path, fn);
    }

    find_removed_files();
    swap_data_containers();
  }

  void poll_monitor::collect_initial_data()
  {
    path_visitor fn = std::bind(&poll_monitor::initial_scan_callback, 
                                this,
                                std::placeholders::_1,
                                std::placeholders::_2);

    for (const string& path : paths)
    {
      scan(path, fn);
    }
  }

  void poll_monitor::run()
  {
    collect_initial_data();

    for (;;)
    {
      std::unique_lock<std::mutex> run_guard(run_mutex);
      if (should_stop) break;
      run_guard.unlock();

      FSW_ELOG(_("Done scanning.\n"));

      sleep(latency < MIN_POLL_LATENCY ? MIN_POLL_LATENCY : latency);

      time(&curr_time);

      collect_data();

      if (!events.empty())
      {
        notify_events(events);
        events.clear();
      }
    }
  }
}
