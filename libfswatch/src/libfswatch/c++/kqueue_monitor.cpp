/*
 * Copyright (c) 2014-2015 Enrico M. Crisostomo
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
#ifdef HAVE_CONFIG_H
#  include "libfswatch_config.h"
#endif

#ifdef HAVE_SYS_EVENT_H

#  include "gettext_defs.h"
#  include "kqueue_monitor.hpp"
#  include "libfswatch_map.hpp"
#  include "libfswatch_set.hpp"
#  include "libfswatch_exception.hpp"
#  include "../c/libfswatch_log.h"
#  include "path_utils.hpp"
#  include <iostream>
#  include <sys/types.h>
#  include <ctime>
#  include <cstdio>
#  include <cmath>
#  include <unistd.h>
#  include <fcntl.h>

namespace fsw
{

  struct kqueue_monitor_load
  {
    fsw_hash_map<std::string, int> descriptors_by_file_name;
    fsw_hash_map<int, std::string> file_names_by_descriptor;
    fsw_hash_map<int, mode_t> file_modes;
    fsw_hash_set<int> descriptors_to_remove;
    fsw_hash_set<int> descriptors_to_rescan;

    void add_watch(int fd, const std::string& path, const struct stat& fd_stat)
    {
      descriptors_by_file_name[path] = fd;
      file_names_by_descriptor[fd] = path;
      file_modes[fd] = fd_stat.st_mode;
    }

    void remove_watch(int fd)
    {
      std::string name = file_names_by_descriptor[fd];
      file_names_by_descriptor.erase(fd);
      descriptors_by_file_name.erase(name);
      file_modes.erase(fd);

      close(fd);
    }

    void remove_watch(const std::string& path)
    {
      int fd = descriptors_by_file_name[path];
      descriptors_by_file_name.erase(path);
      file_names_by_descriptor.erase(fd);
      file_modes.erase(fd);

      close(fd);
    }
  };

  typedef struct KqueueFlagType
  {
    uint32_t flag;
    fsw_event_flag type;
  } KqueueFlagType;

  static std::vector<KqueueFlagType> create_flag_type_vector()
  {
    std::vector<KqueueFlagType> flags;
    flags.push_back({NOTE_DELETE, fsw_event_flag::Removed});
    flags.push_back({NOTE_WRITE, fsw_event_flag::Updated});
    flags.push_back({NOTE_EXTEND, fsw_event_flag::PlatformSpecific});
    flags.push_back({NOTE_ATTRIB, fsw_event_flag::AttributeModified});
    flags.push_back({NOTE_LINK, fsw_event_flag::Link});
    flags.push_back({NOTE_RENAME, fsw_event_flag::Renamed});
    flags.push_back({NOTE_REVOKE, fsw_event_flag::PlatformSpecific});

    return flags;
  }

  static const std::vector<KqueueFlagType> event_flag_type = create_flag_type_vector();

  kqueue_monitor::kqueue_monitor(std::vector<std::string> paths_to_monitor,
                                 FSW_EVENT_CALLBACK *callback,
                                 void *context) :
    monitor(paths_to_monitor, callback, context), load(new kqueue_monitor_load())
  {
  }

  kqueue_monitor::~kqueue_monitor()
  {
    terminate_kqueue();
    delete load;
  }

  static std::vector<fsw_event_flag> decode_flags(uint32_t flag)
  {
    std::vector<fsw_event_flag> evt_flags;

    for (const KqueueFlagType& type : event_flag_type)
    {
      if (flag & type.flag)
      {
        evt_flags.push_back(type.type);
      }
    }

    return evt_flags;
  }

  static struct timespec create_timespec_from_latency(double latency)
  {
    double seconds;
    double nanoseconds = modf(latency, &seconds);
    nanoseconds *= 1000000000;

    struct timespec ts;
    ts.tv_sec = seconds;
    ts.tv_nsec = nanoseconds;

    return ts;
  }

  bool kqueue_monitor::is_path_watched(const std::string& path) const
  {
    return load->descriptors_by_file_name.find(path) != load->descriptors_by_file_name.end();
  }

  bool kqueue_monitor::add_watch(const std::string& path, const struct stat& fd_stat)
  {
    // check if the path is already watched and if it is,
    // skip it and return false.
    if (is_path_watched(path))
    {
      return false;
    }

    int o_flags = 0;
#  ifdef O_SYMLINK
    o_flags |= O_SYMLINK;
#  endif
#  ifdef O_EVTONLY
    // The descriptor is requested for event notifications only.
    o_flags |= O_EVTONLY;
#  else
    o_flags |= O_RDONLY;
#  endif

    int fd = open(path.c_str(), o_flags);

    if (fd == -1)
    {
      fsw_logf_perror(_("Cannot open %s"), path.c_str());

      return false;
    }

    // if the descriptor could be opened, track it
    load->add_watch(fd, path, fd_stat);

    return true;
  }

  bool kqueue_monitor::scan(const std::string& path, bool is_root_path)
  {
    struct stat fd_stat;
    if (!lstat_path(path, fd_stat)) return false;

    if (follow_symlinks && S_ISLNK(fd_stat.st_mode))
    {
      std::string link_path;
      if (read_link_path(path, link_path))
        return scan(link_path);

      return false;
    }

    bool is_dir = S_ISDIR(fd_stat.st_mode);

    if (!is_dir && !is_root_path && directory_only) return true;
    if (!accept_path(path)) return true;
    if (!add_watch(path, fd_stat)) return false;
    if (!recursive) return true;
    if (!is_dir) return true;

    std::vector<std::string> children = get_directory_children(path);

    for (const std::string& child : children)
    {
      if (child == "." || child == "..") continue;

      scan(path + "/" + child, false);
    }

    return true;
  }

  void kqueue_monitor::remove_deleted()
  {
    auto fd = load->descriptors_to_remove.begin();

    while (fd != load->descriptors_to_remove.end())
    {
      load->remove_watch(*fd);
      load->descriptors_to_remove.erase(fd++);
    }
  }

  void kqueue_monitor::rescan_pending()
  {
    auto fd = load->descriptors_to_rescan.begin();

    while (fd != load->descriptors_to_rescan.end())
    {
      std::string fd_path = load->file_names_by_descriptor[*fd];

      // Rescan the hierarchy rooted at fd_path.
      // If the path does not exist any longer, nothing needs to be done since
      // kqueue(2) says:
      //
      // EV_DELETE  Events which are attached to file descriptors are
      //            automatically deleted on the last close of the descriptor.
      //
      // If the descriptor which has vanished is a directory, we don't bother
      // EV_DELETEing all its children the event from kqueue for the same
      // reason.
      load->remove_watch(fd_path);

      scan(fd_path);

      load->descriptors_to_rescan.erase(fd++);
    }
  }

  void kqueue_monitor::scan_root_paths()
  {
    for (std::string& path : paths)
    {
      if (is_path_watched(path)) continue;

      if (!scan(path))
      {
        FSW_ELOGF(_("%s cannot be found. Will retry later.\n"), path.c_str());
      }
    }
  }

  void kqueue_monitor::initialize_kqueue()
  {
    if (kq != -1) throw libfsw_exception(_("kqueue already running."));

    kq = kqueue();

    if (kq == -1)
    {
      perror("kqueue()");
      throw libfsw_exception(_("kqueue failed."));
    }
  }

  void kqueue_monitor::terminate_kqueue()
  {
    if (kq != -1) close(kq);
    kq = -1;
  }

  int kqueue_monitor::wait_for_events(const std::vector<struct kevent>& changes,
                                      std::vector<struct kevent>& event_list)
  {
    struct timespec ts = create_timespec_from_latency(latency);

    int event_num = kevent(kq,
                           &changes[0],
                           (int) changes.size(),
                           &event_list[0],
                           (int) event_list.size(),
                           &ts);

    // Ignore errors when kevent() is interrupted by a signal.
    if (event_num == -1 && errno != EINTR)
    {
      perror("kevent");
      throw libfsw_exception(_("kevent returned -1, invalid event number."));
    }

    return event_num;
  }

  void kqueue_monitor::process_events(const std::vector<struct kevent>& changes,
                                      const std::vector<struct kevent>& event_list,
                                      int event_num)
  {
    time_t curr_time;
    time(&curr_time);
    std::vector<event> events;

    for (auto i = 0; i < event_num; ++i)
    {
      struct kevent e = event_list[i];

      if (e.flags & EV_ERROR)
      {
        perror(_("Event with EV_ERROR"));
        continue;
      }

      // If a NOTE_DELETE is found or a NOTE_LINK is found on a directory, then
      // the descriptor should be closed and the node rescanned: removing a
      // subtree in *BSD usually result in NOTE_REMOVED | NOTE_LINK being logged
      // for each subdirectory, but sometimes NOTE_WRITE | NOTE_LINK is only
      // observed.  For this reason we mark those descriptors as to be deleted
      // anyway.
      //
      // If a NOTE_RENAME or NOTE_REVOKE flag is found, the file
      // descriptor should probably be closed and the file should be rescanned.
      // If a NOTE_WRITE flag is found and the descriptor is a directory, then
      // the directory needs to be rescanned because at least one file has
      // either been created or deleted.
      if ((e.fflags & NOTE_DELETE))
      {
        load->descriptors_to_remove.insert(e.ident);
      }
      else if ((e.fflags & NOTE_RENAME) || (e.fflags & NOTE_REVOKE)
               || ((e.fflags & NOTE_WRITE) && S_ISDIR(load->file_modes[e.ident])))
      {
        load->descriptors_to_rescan.insert(e.ident);
      }

      // Invoke the callback passing every path for which an event has been
      // received with a non empty filter flag.
      if (e.fflags)
      {
        events.push_back({load->file_names_by_descriptor[e.ident],
                          curr_time,
                          decode_flags(e.fflags)});
      }
    }

    if (events.size())
    {
      notify_events(events);
    }
  }

  void kqueue_monitor::run()
  {
    initialize_kqueue();

    for(;;)
    {
#ifdef HAVE_CXX_MUTEX
      std::unique_lock<std::mutex> run_guard(run_mutex);
      if (should_stop) break;
      run_guard.unlock();
#endif

      // remove the deleted descriptors
      remove_deleted();

      // rescan the pending descriptors
      rescan_pending();

      // scan the root paths to check whether someone is missing
      scan_root_paths();

      std::vector<struct kevent> changes;
      std::vector<struct kevent> event_list;

      for (const std::pair<int, std::string>& fd_path : load->file_names_by_descriptor)
      {
        struct kevent change;

        EV_SET(&change,
               fd_path.first,
               EVFILT_VNODE,
               EV_ADD | EV_ENABLE | EV_CLEAR,
               NOTE_DELETE | NOTE_EXTEND | NOTE_RENAME | NOTE_WRITE | NOTE_ATTRIB | NOTE_LINK | NOTE_REVOKE,
               0,
               0);

        changes.push_back(change);
        struct kevent event;
        event_list.push_back(event);
      }

      /*
       * If no files can be observed yet, then wait and repeat the loop.
       */
      if (!changes.size())
      {
        sleep(latency);
        continue;
      }

      const int event_num = wait_for_events(changes, event_list);
      process_events(changes, event_list, event_num);
    }

    terminate_kqueue();
  }
}

#endif  /* HAVE_SYS_EVENT_H */
