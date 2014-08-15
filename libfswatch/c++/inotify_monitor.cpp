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

#include "inotify_monitor.h"
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <ctime>
#include "libfswatch_exception.h"
#include "libfswatch_log.h"
#include "libfswatch_map.h"

using namespace std;

namespace fsw
{

  struct inotify_monitor_load
  {
    int inotify_monitor_handle = -1;
    std::vector<event> events;
    fsw_hash_map<int, std::string> file_names_by_descriptor;
    time_t curr_time;
  };

  static const unsigned int BUFFER_SIZE = (10 * ((sizeof (struct inotify_event)) + NAME_MAX + 1));

  inotify_monitor::inotify_monitor(vector<string> paths_to_monitor,
                                   FSW_EVENT_CALLBACK * callback,
                                   void * context) :
    monitor(paths_to_monitor, callback, context), load(new inotify_monitor_load())
  {
    load->inotify_monitor_handle = ::inotify_init();

    if (load->inotify_monitor_handle == -1)
    {
      ::perror("inotify_init");
      throw libfsw_exception("Cannot initialize inotify.");
    }
  }

  inotify_monitor::~inotify_monitor()
  {
    // close inotify watchers
    for (auto inotify_desc_pair : load->file_names_by_descriptor)
    {
      if (::inotify_rm_watch(load->inotify_monitor_handle, inotify_desc_pair.first))
      {
        ::perror("rm");
      }
    }

    // close inotify
    if (load->inotify_monitor_handle > 0)
    {
      ::close(load->inotify_monitor_handle);
    }

    delete load;
  }

  void inotify_monitor::scan(const string &path)
  {
    if (!accept_path(path)) return;

    int inotify_desc = ::inotify_add_watch(load->inotify_monitor_handle, path.c_str(), IN_ALL_EVENTS);

    if (inotify_desc == -1)
    {
      ::perror("inotify_add_watch");
      throw libfsw_exception("Cannot add watch.");
    }

    load->file_names_by_descriptor[inotify_desc] = path;

    std::ostringstream s;
    s << "Watching " << path << ".\n";

    libfsw_log(s.str().c_str());
  }

  void inotify_monitor::collect_initial_data()
  {
    for (string &path : paths)
    {
      scan(path);
    }
  }

  void inotify_monitor::preprocess_dir_event(struct inotify_event * event)
  {
    vector<fsw_event_flag> flags;

    if (event->mask & IN_DELETE_SELF) flags.push_back(fsw_event_flag::Removed);
    if (event->mask & IN_ISDIR) flags.push_back(fsw_event_flag::IsDir);
    if (event->mask & IN_MOVE_SELF) flags.push_back(fsw_event_flag::Updated);
    if (event->mask & IN_UNMOUNT) flags.push_back(fsw_event_flag::PlatformSpecific);

    if (flags.size())
    {
      load->events.push_back({load->file_names_by_descriptor[event->wd], load->curr_time, flags});
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
    if (event->mask & IN_MOVED_FROM) flags.push_back(fsw_event_flag::Updated);
    if (event->mask & IN_MOVED_TO) flags.push_back(fsw_event_flag::Updated);
    if (event->mask & IN_OPEN) flags.push_back(fsw_event_flag::PlatformSpecific);

    if (flags.size())
    {
      ostringstream path_stream;
      path_stream << load->file_names_by_descriptor[event->wd];

      if (event->len > 1)
      {
        path_stream << "/";
        path_stream << event->name;
      }

      load->events.push_back({path_stream.str(), load->curr_time, flags});
    }
  }

  void inotify_monitor::preprocess_event(struct inotify_event * event)
  {
    if (event->mask & IN_Q_OVERFLOW)
    {
      throw libfsw_exception("Event queue overflowed.");
    }

    preprocess_dir_event(event);
    preprocess_node_event(event);
  }

  void inotify_monitor::notify_events()
  {
    if (load->events.size())
    {
      callback(load->events);
      load->events.clear();
    }
  }

  void inotify_monitor::run()
  {
    collect_initial_data();

    char buffer[BUFFER_SIZE];

    while (true)
    {
      ssize_t record_num = ::read(load->inotify_monitor_handle,
                                  buffer,
                                  BUFFER_SIZE);

      if (!record_num)
      {
        throw libfsw_exception("::read() on inotify descriptor read 0 records.");
      }

      if (record_num == -1)
      {
        ::perror("read()");
        throw libfsw_exception("::read() on inotify descriptor returned -1.");
      }

      time(&load->curr_time);

      for (char *p = buffer; p < buffer + record_num;)
      {
        struct inotify_event * event = reinterpret_cast<struct inotify_event *> (p);

        preprocess_event(event);

        p += (sizeof (struct inotify_event)) + event->len;
      }

      notify_events();
    }
  }
}
