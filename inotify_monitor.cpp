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
#include "inotify_monitor.h"

#ifdef HAVE_SYS_INOTIFY_H

#  include <limits.h>
#  include <unistd.h>
#  include <stdio.h>
#  include <iostream>
#  include <sstream>
#  include "fswatch_exception.h"
#  include "fswatch_log.h"

using namespace std;

static const unsigned int BUFFER_SIZE = (10 * ((sizeof (struct inotify_event)) + NAME_MAX + 1));

inotify_monitor::inotify_monitor(vector<string> paths_to_monitor,
                                 EVENT_CALLBACK callback) :
  monitor(paths_to_monitor, callback)
{
  inotify = ::inotify_init();
  if (inotify == -1)
  {
    ::perror("inotify_init");
    throw fsw_exception("Cannot initialize inotify.");
  }
}

inotify_monitor::~inotify_monitor()
{
  // close inotify watchers
  for (auto inotify_desc_pair : file_names_by_descriptor)
  {
    if (::inotify_rm_watch(inotify, inotify_desc_pair.first))
    {
      ::perror("rm");
    }
  }

  // close inotify
  if (inotify > 0)
  {
    ::close(inotify);
  }
}

void inotify_monitor::scan(const string &path)
{
  if (!accept_path(path)) return;

  int inotify_desc = ::inotify_add_watch(inotify, path.c_str(), IN_ALL_EVENTS);

  if (inotify_desc == -1)
  {
    ::perror("inotify_add_watch");
    throw fsw_exception("Cannot add watch.");
  }

  file_names_by_descriptor[inotify_desc] = path;

  cout << "Watching " << path << "." << endl;
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
  vector<event_flag> flags;

  if (event->mask & IN_DELETE_SELF) flags.push_back(event_flag::Removed);
  if (event->mask & IN_ISDIR) flags.push_back(event_flag::IsDir);
  if (event->mask & IN_MOVE_SELF) flags.push_back(event_flag::Updated);
  if (event->mask & IN_UNMOUNT) flags.push_back(event_flag::PlatformSpecific);

  if (flags.size())
  {
    events.push_back({file_names_by_descriptor[event->wd], curr_time, flags});
  }
}

void inotify_monitor::preprocess_node_event(struct inotify_event * event)
{
  vector<event_flag> flags;

  if (event->mask & IN_ACCESS) flags.push_back(event_flag::PlatformSpecific);
  if (event->mask & IN_ATTRIB) flags.push_back(event_flag::AttributeModified);
  if (event->mask & IN_CLOSE_NOWRITE) flags.push_back(event_flag::PlatformSpecific);
  if (event->mask & IN_CLOSE_WRITE) flags.push_back(event_flag::Updated);
  if (event->mask & IN_CREATE) flags.push_back(event_flag::Created);
  if (event->mask & IN_DELETE) flags.push_back(event_flag::Removed);
  if (event->mask & IN_MODIFY) flags.push_back(event_flag::Updated);
  if (event->mask & IN_MOVED_FROM) flags.push_back(event_flag::Updated);
  if (event->mask & IN_MOVED_TO) flags.push_back(event_flag::Updated);
  if (event->mask & IN_OPEN) flags.push_back(event_flag::PlatformSpecific);

  if (flags.size())
  {
    ostringstream path_stream;
    path_stream << file_names_by_descriptor[event->wd];

    if (event->len > 1)
    {
      path_stream << "/";
      path_stream << event->name;
    }

    events.push_back({path_stream.str(), curr_time, flags});
  }
}

void inotify_monitor::preprocess_event(struct inotify_event * event)
{
  if (event->mask & IN_Q_OVERFLOW)
  {
    throw fsw_exception("Event queue overflowed.");
  }

  preprocess_dir_event(event);
  preprocess_node_event(event);
}

void inotify_monitor::notify_events()
{
  if (events.size())
  {
    callback(events);
    events.clear();
  }
}

void inotify_monitor::run()
{
  collect_initial_data();

  char buffer[BUFFER_SIZE];

  while (true)
  {
    ssize_t record_num = ::read(inotify, buffer, BUFFER_SIZE);

    if (!record_num)
    {
      throw fsw_exception("::read() on inotify descriptor read 0 records.");
    }

    if (record_num == -1)
    {
      ::perror("read()");
      throw fsw_exception("::read() on inotify descriptor returned -1.");
    }

    time(&curr_time);

    for (char *p = buffer; p < buffer + record_num;)
    {
      struct inotify_event * event = reinterpret_cast<struct inotify_event *> (p);

      preprocess_event(event);

      p += (sizeof (struct inotify_event)) + event->len;
    }

    notify_events();
  }
}

#endif  /* HAVE_SYS_INOTIFY_H */
