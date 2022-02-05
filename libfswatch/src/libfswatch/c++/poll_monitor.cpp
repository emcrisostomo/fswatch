/*
 * Copyright (c) 2014-2021 Enrico M. Crisostomo
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
#include "gettext_defs.h"
#include <iostream>
#include <utility>

#ifdef _MSC_VER
#include "libfswatch/libfswatch_config.h"
#ifndef _ARM_
#include <intrin.h>
#endif /* !_ARM_ */
#include <Synchapi.h>

/* stolen from libcurl: */
#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

/* stolen from libcurl: */
#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#else
#  include <unistd.h>
#  include <fcntl.h>
#endif /* _MSC_VER */

#include "libfswatch/libfswatch_config.h"
#include "libfswatch_log.h"
#include "poll_monitor.hpp"
#include "path_utils.hpp"
#include "libfswatch_map.hpp"

#ifdef HAVE_STRUCT_STAT_ST_MTIME
#  define FSW_MTIME(stat) ((stat).st_mtime)
#  define FSW_CTIME(stat) ((stat).st_ctime)
#elif defined(HAVE_STRUCT_STAT_ST_MTIMESPEC)
#  define FSW_MTIME(stat) (stat.st_mtimespec.tv_sec)
#  define FSW_CTIME(stat) (stat.st_ctimespec.tv_sec)
#else
#  error "Either HAVE_STRUCT_STAT_ST_MTIME or HAVE_STRUCT_STAT_ST_MTIMESPEC must be defined"
#endif

namespace fsw
{
  using std::vector;
  using std::string;

  using poll_monitor_data = struct poll_monitor::poll_monitor_data
  {
    fsw_hash_map<string, poll_monitor::watched_file_info> tracked_files;
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

    watched_file_info wfi{FSW_MTIME(stat), FSW_CTIME(stat)};
    new_data->tracked_files[path] = wfi;

    if (previous_data->tracked_files.count(path))
    {
      watched_file_info pwfi = previous_data->tracked_files[path];
      vector<fsw_event_flag> flags;

      if (FSW_MTIME(stat) > pwfi.mtime)
      {
        flags.push_back(fsw_event_flag::Updated);
      }

      if (FSW_CTIME(stat) > pwfi.ctime)
      {
        flags.push_back(fsw_event_flag::AttributeModified);
      }

      if (!flags.empty())
      {
        events.emplace_back(path, curr_time, flags);
      }

      previous_data->tracked_files.erase(path);
    }
    else
    {
      vector<fsw_event_flag> flags;
      flags.push_back(fsw_event_flag::Created);
      events.emplace_back(path, curr_time, flags);
    }

    return true;
  }

  bool poll_monitor::add_path(const string& path,
                              const struct stat& fd_stat,
                              poll_monitor_scan_callback poll_callback)
  {
    return ((*this).*(poll_callback))(path, fd_stat);
  }

  void poll_monitor::scan(const string& path, poll_monitor_scan_callback fn)
  {
    struct stat fd_stat;
    if (!lstat_path(path, fd_stat)) return;

#ifdef _MSC_VER
#define _ISLNK !S_ISREG
#else
#define _ISLNK S_ISLNK
#endif /* _MSC_VER */

    if (follow_symlinks && _ISLNK(fd_stat.st_mode))
    {
      string link_path;
      if (read_link_path(path, link_path))
        scan(link_path, fn);

      return;
    }
#undef _ISLNK

    if (!accept_path(path)) return;
    if (!add_path(path, fd_stat, fn)) return;
    if (!recursive) return;
    if (!S_ISDIR(fd_stat.st_mode)) return;

    vector<string> children = get_directory_children(path);

    for (const string& child : children)
    {
      if (child == "." || child == "..") continue;

      std::string new_path = path;
      new_path += PATH_SEP;
      new_path += child;
      scan(new_path, fn);
    }
  }

  void poll_monitor::find_removed_files()
  {
    vector<fsw_event_flag> flags;
    flags.push_back(fsw_event_flag::Removed);

    for (const auto& removed : previous_data->tracked_files)
    {
      events.emplace_back(removed.first, curr_time, flags);
    }
  }

  void poll_monitor::swap_data_containers()
  {
    previous_data = std::move(new_data);
    new_data.reset(new poll_monitor_data());
  }

  void poll_monitor::collect_data()
  {
    poll_monitor_scan_callback fn = &poll_monitor::intermediate_scan_callback;

    for (const string& path : paths)
    {
      scan(path, fn);
    }

    find_removed_files();
    swap_data_containers();
  }

  void poll_monitor::collect_initial_data()
  {
    poll_monitor_scan_callback fn = &poll_monitor::initial_scan_callback;

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
#ifdef HAVE_CXX_MUTEX
      std::unique_lock<std::mutex> run_guard(run_mutex);
      if (should_stop) break;
      run_guard.unlock();
#endif

      FSW_ELOG(_("Done scanning.\n"));

#ifdef _MSC_VER
      Sleep( 1000L * static_cast<DWORD>(
#else
     sleep( (
#endif
            (latency < MIN_POLL_LATENCY ? MIN_POLL_LATENCY : latency)));

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
