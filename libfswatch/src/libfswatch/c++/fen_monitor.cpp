/*
 * Copyright (c) 2015 Enrico M. Crisostomo
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

#ifdef HAVE_PORT_H

#  include <time.h>
#  include <cerrno>
#  include <cmath>
#  include <cstring>
#  include <cstdlib>
#  include <unistd.h>
#  include <port.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include "gettext_defs.h"
#  include "fen_monitor.hpp"
#  include "libfswatch_map.hpp"
#  include "libfswatch_set.hpp"
#  include "libfswatch_exception.hpp"
#  include "../c/libfswatch_log.h"
#  include "path_utils.hpp"

using namespace std;

namespace fsw
{
  struct fen_info
  {
    struct file_obj fobj;
    int events;
  };

  struct fen_monitor_load
  {
    int port;
    fsw_hash_map<string, struct fen_info *> descriptors_by_file_name;
    fsw_hash_set<struct fen_info *> descriptors_to_remove;
    fsw_hash_set<string> paths_to_rescan;

    void initialize_fen()
    {
      if ((port = port_create()) == -1)
      {
        perror("port_create");
        throw libfsw_exception(_("An error occurred while creating a port."));
      }
    }

    ~fen_monitor_load()
    {
      close_fen();
    }

    void close_fen()
    {
      if (!port) close(port);
      port = 0;
    }

    void add_watch(struct fen_info *fd, const string& path, const struct stat& fd_stat)
    {
      descriptors_by_file_name[path] = fd;
    }

    void remove_watch(const string& path)
    {
      struct fen_info *finfo = descriptors_by_file_name[path];

      if (finfo)
      {
        free(finfo->fobj.fo_name);
        free(finfo);
      }

      descriptors_by_file_name.erase(path);
    }

    struct fen_info * get_descriptor_by_name(const string& path)
    {
      auto res = descriptors_by_file_name.find(path);

      if (res == descriptors_by_file_name.end()) return nullptr;

      return res->second;
    }
  };

  typedef struct FenFlagType
  {
    uint32_t flag;
    fsw_event_flag type;
  } FenFlagType;

  static vector<FenFlagType> create_flag_type_vector()
  {
    vector<FenFlagType> flags;
    flags.push_back({FILE_ACCESS,      fsw_event_flag::PlatformSpecific});
    flags.push_back({FILE_MODIFIED,    fsw_event_flag::Updated});
    flags.push_back({FILE_ATTRIB,      fsw_event_flag::AttributeModified});
    flags.push_back({FILE_DELETE,      fsw_event_flag::Removed});
    flags.push_back({FILE_RENAME_TO,   fsw_event_flag::MovedTo});
    flags.push_back({FILE_RENAME_FROM, fsw_event_flag::MovedFrom});
    flags.push_back({FILE_TRUNC,       fsw_event_flag::PlatformSpecific});
    flags.push_back({UNMOUNTED,        fsw_event_flag::PlatformSpecific});
    flags.push_back({MOUNTEDOVER,      fsw_event_flag::PlatformSpecific});

    return flags;
  }

  static const vector<FenFlagType> event_flag_type = create_flag_type_vector();

  fen_monitor::fen_monitor(vector<string> paths_to_monitor,
                           FSW_EVENT_CALLBACK * callback,
                           void * context) :
    monitor(paths_to_monitor, callback, context), load(new fen_monitor_load())
  {
  }

  fen_monitor::~fen_monitor()
  {
    delete load;
  }

  static vector<fsw_event_flag> decode_flags(uint32_t flag)
  {
    vector<fsw_event_flag> evt_flags;

    for (const FenFlagType &type : event_flag_type)
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

  static int timespec_subtract(timespec& result, timespec& x, timespec& y)
  {
    /* Perform the carry for the later subtraction by updating y. */
    if (x.tv_nsec < y.tv_nsec)
    {
      int nsec = (y.tv_nsec - x.tv_nsec) / 1000000000 + 1;
      y.tv_nsec -= 1000000000 * nsec;
      y.tv_sec += nsec;
    }

    if (x.tv_nsec - y.tv_nsec > 1000000000)
    {
      int nsec = (x.tv_nsec - y.tv_nsec) / 1000000000;
      y.tv_nsec += 1000000000 * nsec;
      y.tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_nsec is certainly positive. */
    result.tv_sec = x.tv_sec - y.tv_sec;
    result.tv_nsec = x.tv_nsec - y.tv_nsec;

    /* Return 1 if result is negative. */
    return x.tv_sec < y.tv_sec;
  }

  bool fen_monitor::associate_port(struct fen_info *finfo, const struct stat &fd_stat)
  {
    FSW_ELOGF(_("Associating %s.\n"), finfo->fobj.fo_name);

    struct file_obj *fobjp = &finfo->fobj;
    finfo->fobj.fo_atime = fd_stat.st_atim;
    finfo->fobj.fo_mtime = fd_stat.st_mtim;
    finfo->fobj.fo_ctime = fd_stat.st_ctim;

    if (port_associate(load->port,
                       PORT_SOURCE_FILE,
                       reinterpret_cast<uintptr_t>(fobjp),
                       finfo->events,
                       static_cast<void *>(finfo)) != 0)
    {
      // File may have been deleted or moved while processing the event.
      perror("port_associate()");
      load->remove_watch(finfo->fobj.fo_name);
      return false;
    }

    return true;
  }

  bool fen_monitor::add_watch(const string &path, const struct stat &fd_stat)
  {
    // check if the path is already watched and if it is,
    // skip it and return false.
    if (is_path_watched(path))
    {
      return false;
    }

    FSW_ELOGF(_("Adding %s to list of watched paths.\n"), path.c_str());

    struct fen_info *finfo = load->get_descriptor_by_name(path);

    if (finfo == nullptr)
    {
      FSW_ELOG(_("Allocating fen_info.\n"));

      finfo = static_cast<struct fen_info *>(malloc(sizeof(struct fen_info)));

      if (!finfo)
      {
        perror("malloc()");
        throw libfsw_exception(_("Cannot allocate memory"));
      }

      if (!(finfo->fobj.fo_name = strdup(path.c_str())))
      {
        free(finfo);
        perror("strdup()");
        throw libfsw_exception(_("Cannot allocate memory"));
      }
    }

    // FILE_NOFOLLOW is currently not used because links are followed manually.
    finfo->events = FILE_MODIFIED | FILE_ATTRIB | FILE_TRUNC;
    if (watch_access) finfo->events |= FILE_ACCESS;
    if (!follow_symlinks) finfo->events |= FILE_NOFOLLOW;

    // if the descriptor could be opened, track it
    if (associate_port(finfo, fd_stat)) { load->add_watch(finfo, path, fd_stat); }

    return true;
  }

  bool fen_monitor::is_path_watched(const string& path) const
  {
    return (load->descriptors_by_file_name.find(path) != load->descriptors_by_file_name.end())
      && (load->paths_to_rescan.find(path) == load->paths_to_rescan.end());
  }

  bool fen_monitor::scan(const string& path, bool is_root_path)
  {
    struct stat fd_stat;
    if (!stat_path(path, fd_stat))
    {
      load->remove_watch(path);
      return false;
    }

    bool is_dir = S_ISDIR(fd_stat.st_mode);

    if (!is_dir && !is_root_path && directory_only) return true;
    if (!is_dir && !accept_path(path)) return true;
    if (!is_dir) return add_watch(path, fd_stat);
    if (!recursive) return true;

    vector<string> children = get_directory_children(path);

    for (string& child : children)
    {
      if (child.compare(".") == 0 || child.compare("..") == 0) continue;

      scan(path + "/" + child, false);
    }

    return add_watch(path, fd_stat);
  }

  void fen_monitor::scan_root_paths()
  {
    for (string& path : paths)
    {
      if (is_path_watched(path)) continue;

      if (!scan(path))
      {
        FSW_ELOGF(_("%s cannot be found. Will retry later.\n"), path.c_str());
      }
    }
  }

  void fen_monitor::process_events(struct fen_info *finfo, int event_flags)
  {
    time_t curr_time;
    time(&curr_time);

    vector<event> events;
    events.push_back({finfo->fobj.fo_name, curr_time, decode_flags(event_flags)});

    // The File Events Notification API requires the caller to associate a file path
    // each time an event is retrieved.
    if (event_flags & FILE_DELETE) load->descriptors_to_remove.insert(finfo);
    else load->paths_to_rescan.insert(finfo->fobj.fo_name);

    notify_events(events);
  }

  void fen_monitor::rescan_removed()
  {
    FSW_ELOG(_("Processing deleted descriptors.\n"));

    auto fd = load->descriptors_to_remove.begin();

    while (fd != load->descriptors_to_remove.end())
    {
      struct fen_info *finfo = *fd;

      if (!port_dissociate(load->port,
                           PORT_SOURCE_FILE,
                           reinterpret_cast<uintptr_t>(&finfo->fobj)) != 0)
      {
        perror("port_dissociate()");
      }

      load->remove_watch(finfo->fobj.fo_name);
      load->descriptors_to_remove.erase(fd++);
    }
  }

  void fen_monitor::rescan_pending()
  {
    FSW_ELOG(_("Rescanning pending descriptors.\n"));

    auto path = load->paths_to_rescan.begin();

    while (path != load->paths_to_rescan.end())
    {
      FSW_ELOGF(_("Rescanning %s.\n"), path->c_str());

      scan(*path);

      load->paths_to_rescan.erase(path++);
    }
  }

  void fen_monitor::run()
  {
    load->initialize_fen();

    double sec;
    double frac = modf(latency, &sec);

    for (;;)
    {
#ifdef HAVE_CXX_MUTEX
      unique_lock<mutex> run_guard(run_mutex);
      if (should_stop) break;
      run_guard.unlock();
#endif

      rescan_removed();
      rescan_pending();

      scan_root_paths();

      port_event_t pe;
      struct timespec timeout;
      timeout.tv_sec = sec;
      timeout.tv_nsec = 1000 * 1000 * 1000 * frac;

      if (port_get(load->port, &pe, &timeout) == 0)
      {
        switch (pe.portev_source)
        {
        case PORT_SOURCE_FILE:
          // Process file events.
          process_events((struct fen_info *)pe.portev_object, pe.portev_events);
          break;
        default:
          const char *msg = _("Event from unexpected source");
          perror(msg);
          throw libfsw_exception(msg);
        }
      }
      else if (errno != ETIME && errno != EINTR) perror("port_get");
    }
  }
}

#endif  /* HAVE_PORT_H */
