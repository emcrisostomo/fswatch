/* 
 * Copyright (C) 2014, Enrico M. Crisostomo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#  include "libfswatch_config.h"
#endif

#include "gettext_defs.h"
#include "inotify_monitor.h"
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <sstream>
#include <ctime>
#include <cmath>
#include <sys/select.h>
#include "libfswatch_exception.h"
#include "../c/libfswatch_log.h"
#include "libfswatch_map.h"
#include "libfswatch_set.h"
#include "path_utils.h"

using namespace std;

namespace fsw
{

  struct inotify_monitor_impl
  {
    int inotify_monitor_handle = -1;
    vector<event> events;
    /*
     * A map of file names by descriptor is kept in sync because the name field
     * of the inotify_event structure is present only when it identifies a
     * child of a watched directory.  In all the other cases, we store the path
     * for easy retrieval.
     */
    fsw_hash_set<int> watched_descriptors;
    fsw_hash_map<string, int> path_to_wd;
    /*
     * Since the inotify API maintains only works with watch
     * descriptors a cache maintaining a relationship between a watch
     * descriptor and the path used to get it is required to be able to map an
     * event to the path it refers to.  From man inotify:
     * 
     *   The inotify API identifies events via watch descriptors.  It is the
     *   application's responsibility to cache a mapping (if one is needed)
     *   between watch descriptors and pathnames.  Be aware that directory
     *   renamings may affect multiple cached pathnames.
     */
    fsw_hash_map<int, string> wd_to_path;
    fsw_hash_set<int> descriptors_to_remove;
    fsw_hash_set<int> watches_to_remove;
    time_t curr_time;
  };

  static const unsigned int BUFFER_SIZE = (10 * ((sizeof (struct inotify_event)) + NAME_MAX + 1));

  REGISTER_MONITOR_IMPL(inotify_monitor, inotify_monitor_type);

  inotify_monitor::inotify_monitor(vector<string> paths_to_monitor,
                                   FSW_EVENT_CALLBACK * callback,
                                   void * context) :
    monitor(paths_to_monitor, callback, context),
    impl(new inotify_monitor_impl())
  {
    impl->inotify_monitor_handle = ::inotify_init();

    if (impl->inotify_monitor_handle == -1)
    {
      ::perror("inotify_init");
      throw libfsw_exception(_("Cannot initialize inotify."));
    }
  }

  inotify_monitor::~inotify_monitor()
  {
    // close inotify watchers
    for (auto inotify_desc_pair : impl->watched_descriptors)
    {
      ostringstream log;
      log << _("Removing: ") << inotify_desc_pair << "\n";
      libfsw_log(log.str().c_str());

      if (::inotify_rm_watch(impl->inotify_monitor_handle, inotify_desc_pair))
      {
        ::perror("inotify_rm_watch");
      }
    }

    // close inotify
    if (impl->inotify_monitor_handle > 0)
    {
      ::close(impl->inotify_monitor_handle);
    }

    delete impl;
  }

  bool inotify_monitor::add_watch(const string &path,
                                  const struct stat &fd_stat)
  {
    // TODO: Consider optionally adding the IN_EXCL_UNLINK flag.
    int inotify_desc = ::inotify_add_watch(impl->inotify_monitor_handle,
                                           path.c_str(),
                                           IN_ALL_EVENTS);

    if (inotify_desc == -1)
    {
      ::perror("inotify_add_watch");
    }
    else
    {
      impl->watched_descriptors.insert(inotify_desc);
      impl->wd_to_path[inotify_desc] = path;
      impl->path_to_wd[path] = inotify_desc;

      ostringstream log;
      log << _("Added: ") << path << "\n";
      libfsw_log(log.str().c_str());
    }

    return (inotify_desc != -1);
  }

  void inotify_monitor::scan(const string &path, const bool accept_non_dirs)
  {
    struct stat fd_stat;
    if (!stat_path(path, fd_stat)) return;

    /*
     * When watching a directory the inotify API will return change events of
     * first-level children.  Therefore, we do not need to manually add a watch
     * for a child unless it is a directory.  By default, accept_non_dirs is
     * true to allow watching a file when first invoked on a node.
     */
    if (!accept_non_dirs && !S_ISDIR(fd_stat.st_mode)) return;
    else if (follow_symlinks && S_ISLNK(fd_stat.st_mode))
    {
      string link_path;
      if (read_link_path(path, link_path))
        scan(link_path/*, fn */, accept_non_dirs);

      return;
    }

    if (!S_ISDIR(fd_stat.st_mode) && !accept_path(path)) return;
    if (!add_watch(path, fd_stat /*, fn */)) return;
    if (!recursive || !S_ISDIR(fd_stat.st_mode)) return;

    vector<string> children;
    get_directory_children(path, children);

    for (const string &child : children)
    {
      if (child.compare(".") == 0 || child.compare("..") == 0) continue;

      /*
       * Scan children but only watch directories.
       */
      scan(path + "/" + child /*, fn */, false);
    }
  }

  bool inotify_monitor::is_watched(const string & path)
  {
    return (impl->path_to_wd.find(path) != impl->path_to_wd.end());
  }

  void inotify_monitor::scan_root_paths()
  {
    for (string &path : paths)
    {
      if (!is_watched(path)) scan(path);
    }
  }

  void inotify_monitor::preprocess_dir_event(struct inotify_event * event)
  {
    vector<fsw_event_flag> flags;

    if (event->mask & IN_ISDIR) flags.push_back(fsw_event_flag::IsDir);
    if (event->mask & IN_MOVE_SELF) flags.push_back(fsw_event_flag::Updated);
    if (event->mask & IN_UNMOUNT) flags.push_back(fsw_event_flag::PlatformSpecific);

    if (flags.size())
    {
      impl->events.push_back({impl->wd_to_path[event->wd], impl->curr_time, flags});
    }
  }

  void inotify_monitor::preprocess_node_event(struct inotify_event * event)
  {
    vector<fsw_event_flag> flags;

    if (event->mask & IN_ACCESS) flags.push_back(fsw_event_flag::PlatformSpecific);
    if (event->mask & IN_ATTRIB) flags.push_back(fsw_event_flag::AttributeModified);
    if (event->mask & IN_CLOSE_NOWRITE) flags.push_back(fsw_event_flag::PlatformSpecific);
    if (event->mask & IN_CLOSE_WRITE) flags.push_back(fsw_event_flag::Updated);
    if (event->mask & IN_CREATE) flags.push_back(fsw_event_flag::Created);
    if (event->mask & IN_DELETE) flags.push_back(fsw_event_flag::Removed);
    if (event->mask & IN_MODIFY) flags.push_back(fsw_event_flag::Updated);
    if (event->mask & IN_MOVED_FROM)
    {
      flags.push_back(fsw_event_flag::Removed);
      flags.push_back(fsw_event_flag::MovedFrom);
    }
    if (event->mask & IN_MOVED_TO)
    {
      flags.push_back(fsw_event_flag::Created);
      flags.push_back(fsw_event_flag::MovedTo);
    }
    if (event->mask & IN_OPEN) flags.push_back(fsw_event_flag::PlatformSpecific);

    // Build the file name.
    ostringstream filename_stream;
    filename_stream << impl->wd_to_path[event->wd];

    if (event->len > 1)
    {
      filename_stream << "/";
      filename_stream << event->name;
    }

    if (flags.size())
    {
      impl->events.push_back({filename_stream.str(), impl->curr_time, flags});
    }

    {
      ostringstream log;
      log << _("Generic event: ") << event->wd << "::" << filename_stream.str() << "\n";
      libfsw_log(log.str().c_str());
    }

    /*
     * inotify automatically removes the watch of a watched item that has been
     * removed and posts an IN_IGNORED event after an IN_DELETE_SELF.
     */
    if (event->mask & IN_IGNORED)
    {
      ostringstream log;
      log << "IN_IGNORED: " << event->wd << "::" << filename_stream.str() << "\n";
      libfsw_log(log.str().c_str());

      impl->descriptors_to_remove.insert(event->wd);
    }

    /*
     * inotify sends an IN_MOVE_SELF event when a watched object is moved into
     * the same filesystem and keeps watching it.  Since its path has changed,
     * we remove the watch so that recreation is attempted at the next
     * iteration.
     * 
     * Beware that a race condition exists which may result in events go
     * unnoticed when a watched file x is removed and a new file named x is
     * created thereafter.  In this case, fswatch could be blocked on ::read
     * and it would not have any chance to create a new watch descriptor for x
     * until an event is received and ::read unblocks.
     */
    if (event->mask & IN_MOVE_SELF)
    {
      ostringstream log;
      log << "IN_MOVE_SELF: " << event->wd << "::" << filename_stream.str() << "\n";
      libfsw_log(log.str().c_str());

      impl->watches_to_remove.insert(event->wd);
      impl->descriptors_to_remove.insert(event->wd);
    }

    /*
     * An file could be moved to a path which is being observed.  The clobbered
     * file is handled by the corresponding IN_DELETE_SELF event.
     */

    /*
     * inotify automatically removes the watch of the object the IN_DELETE_SELF
     * event is related to.
     */
    if (event->mask & IN_DELETE_SELF)
    {
      ostringstream log;
      log << "IN_DELETE_SELF: " << event->wd << "::" << filename_stream.str() << "\n";
      libfsw_log(log.str().c_str());

      impl->descriptors_to_remove.insert(event->wd);
    }
  }

  void inotify_monitor::preprocess_event(struct inotify_event * event)
  {
    if (event->mask & IN_Q_OVERFLOW)
    {
      throw libfsw_exception(_("Event queue overflowed."));
    }

    preprocess_dir_event(event);
    preprocess_node_event(event);
  }

  void inotify_monitor::notify_events()
  {
    if (impl->events.size())
    {
      ostringstream log;
      log << _("Notifying events #: ") << impl->events.size() << "\n";
      libfsw_log(log.str().c_str());

      callback(impl->events, context);
      impl->events.clear();
    }
  }

  void inotify_monitor::remove_watch(int wd)
  {
    /*
     * No need to remove the inotify watch because it is removed automatically
     * when a watched element is deleted. 
     */
    impl->wd_to_path.erase(wd);
  }

  void inotify_monitor::process_pending_events()
  {
    // Remove watches.
    auto wtd = impl->watches_to_remove.begin();

    while (wtd != impl->watches_to_remove.end())
    {
      if (::inotify_rm_watch(impl->inotify_monitor_handle, *wtd) != 0)
      {
        ::perror("inotify_rm_watch");
      }
      else
      {
        ostringstream log;
        log << _("Removed: ") << *wtd << "\n";
        libfsw_log(log.str().c_str());
      }

      impl->watches_to_remove.erase(wtd++);
    }

    // Clean up descriptors.
    auto fd = impl->descriptors_to_remove.begin();

    while (fd != impl->descriptors_to_remove.end())
    {
      const string & curr_path = impl->wd_to_path[*fd];
      impl->path_to_wd.erase(curr_path);
      impl->wd_to_path.erase(*fd);
      impl->watched_descriptors.erase(*fd);

      impl->descriptors_to_remove.erase(fd++);
    }
  }

  void inotify_monitor::run()
  {
    char buffer[BUFFER_SIZE];

    while (true)
    {
      process_pending_events();

      scan_root_paths();

      // If no files can be watched, sleep and repeat the loop.
      if (!impl->watched_descriptors.size())
      {
        ::sleep(latency);
        continue;
      }

      // Use select to timeout on file descriptor read the amount specified by
      // the monitor latency.  This way, the monitor has a chance to update its
      // watches with at least the periodicity expected by the user.
      fd_set set;
      struct timeval timeout;

      FD_ZERO(&set);
      FD_SET(impl->inotify_monitor_handle, &set);
      double sec;
      double frac = ::modf(this->latency, &sec);
      timeout.tv_sec = sec;
      timeout.tv_usec = 1000 * 1000 * frac;

      int rv = ::select(impl->inotify_monitor_handle + 1,
                        &set,
                        nullptr,
                        nullptr,
                        &timeout);

      if (rv == -1)
      {
        throw libfsw_exception(_("::select() on inotify descriptor encountered an error."));
      }

      // In case of read timeout just repeat the loop.
      if (rv == 0)
      {
        continue;
      }

      ssize_t record_num = ::read(impl->inotify_monitor_handle,
                                  buffer,
                                  BUFFER_SIZE);

      {
        ostringstream log;
        log << _("Number of records: ") << record_num << "\n";
        libfsw_log(log.str().c_str());
      }

      if (!record_num)
      {
        throw libfsw_exception(_("::read() on inotify descriptor read 0 records."));
      }

      if (record_num == -1)
      {
        ::perror("read()");
        throw libfsw_exception(_("::read() on inotify descriptor returned -1."));
      }

      time(&impl->curr_time);

      for (char *p = buffer; p < buffer + record_num;)
      {
        struct inotify_event * event = reinterpret_cast<struct inotify_event *> (p);

        preprocess_event(event);

        p += (sizeof (struct inotify_event)) + event->len;
      }

      notify_events();
      ::sleep(latency);
    }
  }
}
