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
  struct fen_monitor_load
  {
    int port;
    fsw_hash_map<string, struct fen_info *> descriptors_by_file_name;
    fsw_hash_map<struct fen_info *, string> file_names_by_descriptor;
    fsw_hash_map<struct fen_info *, mode_t> file_modes;
    fsw_hash_set<struct fen_info *> descriptors_to_remove;
    fsw_hash_set<struct fen_info *> descriptors_to_rescan;

    void initialize_fen()
    {
      if ((port = port_create()) == -1)
      {
        perror("port_create");
        throw libfsw_exception(_("An error occurred while creating a port."));
      }
    }

    void close_fen()
    {
      close(port);
    }

    void add_watch(struct fen_info * fd, const string & path, const struct stat &fd_stat)
    {
      descriptors_by_file_name[path] = fd;
      file_names_by_descriptor[fd] = path;
      file_modes[fd] = fd_stat.st_mode;
    }
  };

  struct fen_info
  {
    struct file_obj fobj;
    int events;
  };

  typedef struct FenFlagType
  {
    uint32_t flag;
    fsw_event_flag type;
  } FenFlagType;

  static vector<FenFlagType> create_flag_type_vector()
  {
    vector<FenFlagType> flags;
    flags.push_back({FILE_ACCESS, fsw_event_flag::PlatformSpecific});
    flags.push_back({FILE_MODIFIED, fsw_event_flag::Updated});
    flags.push_back({FILE_ATTRIB, fsw_event_flag::AttributeModified});
    flags.push_back({FILE_DELETE, fsw_event_flag::Removed});
    flags.push_back({FILE_ACCESS, fsw_event_flag::PlatformSpecific});
    flags.push_back({FILE_RENAME_TO, fsw_event_flag::MovedTo});
    flags.push_back({FILE_RENAME_FROM, fsw_event_flag::MovedFrom});
    flags.push_back({UNMOUNTED, fsw_event_flag::PlatformSpecific});
    flags.push_back({MOUNTEDOVER, fsw_event_flag::PlatformSpecific});

    return flags;
  }

  static const vector<FenFlagType> event_flag_type = create_flag_type_vector();

  REGISTER_MONITOR_IMPL(fen_monitor, fen_monitor_type);

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

  static int timespec_subtract (timespec &result, timespec &x, timespec &y)
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

  bool fen_monitor::add_watch(const string &path, const struct stat &fd_stat)
  {
    // check if the path is already watched and if it is,
    // skip it and return false.
    if (is_path_watched(path))
    {
      return false;
    }

    struct fen_info *finfo = static_cast<struct fen_info *>(malloc(sizeof(struct fen_info)));
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

    // FILE_NOFOLLOW is currently not used because links are followed manually.
    finfo->events = FILE_MODIFIED | FILE_ATTRIB | FILE_ACCESS;

    struct file_obj *fobjp = &finfo->fobj;
    finfo->fobj.fo_atime = fd_stat.st_atim;
    finfo->fobj.fo_mtime = fd_stat.st_mtim;
    finfo->fobj.fo_ctime = fd_stat.st_ctim;

    if (port_associate(load->port,
                       PORT_SOURCE_FILE,
                       (uintptr_t)fobjp,
                       finfo->events,
                       static_cast<void *>(finfo)) != 0)
    {
      // Add error processing as required, file may have been
      // deleted or moved.
      perror("Failed to register file");
      free(finfo->fobj.fo_name);
      free(finfo);

      throw libfsw_exception(_("Could not associate port."));
    }

    // if the descriptor could be opened, track it
    load->add_watch(finfo, path, fd_stat);

    return true;
  }

  bool fen_monitor::is_path_watched(const string & path) const
  {
    return load->descriptors_by_file_name.find(path) != load->descriptors_by_file_name.end();
  }

  bool fen_monitor::scan(const string &path, bool is_root_path)
  {
    struct stat fd_stat;
    if (!stat_path(path, fd_stat)) return false;

    if (follow_symlinks && S_ISLNK(fd_stat.st_mode))
    {
      string link_path;
      if (read_link_path(path, link_path))
        return scan(link_path);

      return false;
    }

    bool is_dir = S_ISDIR(fd_stat.st_mode);

    if (!is_dir && !is_root_path && directory_only) return true;
    if (!is_dir && !accept_path(path)) return true;
    if (!add_watch(path, fd_stat)) return false;
    if (!recursive) return true;
    if (!is_dir) return true;

    vector<string> children = get_directory_children(path);

    for (string & child : children)
    {
      if (child.compare(".") == 0 || child.compare("..") == 0) continue;

      scan(path + "/" + child, false);
    }

    return true;
  }

  void fen_monitor::scan_root_paths()
  {
    for (string &path : paths)
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

    notify_events(events);
  }

  void fen_monitor::run()
  {
    load->initialize_fen();

    while (true)
    {
      scan_root_paths();

      port_event_t pe;
      if (port_get(load->port, &pe, nullptr) == 0)
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
    }
  }
}

#endif  /* HAVE_PORT_H */
