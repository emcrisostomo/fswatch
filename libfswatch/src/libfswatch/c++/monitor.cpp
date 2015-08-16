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
#include "gettext_defs.h"
#include "monitor.hpp"
#include "libfswatch_exception.hpp"
#include "../c/libfswatch_log.h"
#include <cstdlib>
#include <regex.h>
#include <iostream>
#include <sstream>
/*
 * Conditionally include monitor headers for default construction.
 */
#if defined(HAVE_FSEVENTS_FILE_EVENTS)
#  include "fsevents_monitor.hpp"
#endif
#if defined(HAVE_SYS_EVENT_H)
#  include "kqueue_monitor.hpp"
#endif
#if defined(HAVE_SYS_INOTIFY_H)
#  include "inotify_monitor.hpp"
#endif
#include "poll_monitor.hpp"

using namespace std;

namespace fsw
{
  struct compiled_monitor_filter
  {
    regex_t regex;
    fsw_filter_type type;
  };

  monitor::monitor(vector<string> paths,
                   FSW_EVENT_CALLBACK * callback,
                   void * context) :
    paths(paths), callback(callback), context(context)
  {
    if (callback == nullptr)
    {
      throw libfsw_exception(_("Callback cannot be null."), FSW_ERR_CALLBACK_NOT_SET);
    }
  }

  void monitor::set_latency(double latency)
  {
    if (latency < 0)
    {
      throw libfsw_exception(_("Latency cannot be negative."), FSW_ERR_INVALID_LATENCY);
    }

    this->latency = latency;
  }

  void monitor::set_recursive(bool recursive)
  {
    this->recursive = recursive;
  }

  void monitor::add_event_type_filter(const fsw_event_type_filter &filter)
  {
    this->event_type_filters.push_back(filter);
  }

  void monitor::set_event_type_filters(const std::vector<fsw_event_type_filter> &filters)
  {
    event_type_filters.clear();

    for (const auto & filter : filters) add_event_type_filter(filter);
  }

  void monitor::add_filter(const monitor_filter &filter)
  {
    regex_t regex;
    int flags = 0;

    if (!filter.case_sensitive) flags |= REG_ICASE;
    if (filter.extended) flags |= REG_EXTENDED;

    if (::regcomp(&regex, filter.text.c_str(), flags))
    {
      string err = _("An error occurred during the compilation of ") + filter.text;
      throw libfsw_exception(err, FSW_ERR_INVALID_REGEX);
    }

    this->filters.push_back({regex, filter.type});
  }

  void monitor::set_filters(const vector<monitor_filter> &filters)
  {
    for (const monitor_filter &filter : filters)
    {
      add_filter(filter);
    }
  }

  void monitor::set_follow_symlinks(bool follow)
  {
    follow_symlinks = follow;
  }

  bool monitor::accept_event_type(fsw_event_flag event_type) const
  {
    // If no filters are set, then accept the event.
    if (event_type_filters.size() == 0) return true;

    // If filters are set, accept the event only if present amongst the filters.
    for (const auto & filter : event_type_filters)
    {
      if (filter.flag == event_type)
      {
        return true;
      }
    }

    // If no filters match, then reject the event.
    return false;
  }

  bool monitor::accept_path(const string &path) const
  {
    return accept_path(path.c_str());
  }

  bool monitor::accept_path(const char *path) const
  {
    bool is_excluded = false;

    for (const auto &filter : filters)
    {
      if (::regexec(&filter.regex, path, 0, nullptr, 0) == 0)
      {
        if (filter.type == fsw_filter_type::filter_include) return true;

        is_excluded = (filter.type == fsw_filter_type::filter_exclude);
      }
    }

    if (is_excluded) return false;

    return true;
  }

  void * monitor::get_context() const
  {
    return context;
  }

  void monitor::set_context(void * context)
  {
    this->context = context;
  }

  monitor::~monitor()
  {
    for (auto &re : filters)
    {
      ::regfree(&re.regex);
    }

    filters.clear();
  }

  static monitor * create_default_monitor(vector<string> paths,
                                          FSW_EVENT_CALLBACK * callback,
                                          void * context)
  {
#if defined(HAVE_FSEVENTS_FILE_EVENTS)
    return new fsevents_monitor(paths, callback, context);
#elif defined(HAVE_SYS_EVENT_H)
    return new kqueue_monitor(paths, callback, context);
#elif defined(HAVE_SYS_INOTIFY_H)
    return new inotify_monitor(paths, callback, context);
#else
    return new poll_monitor(paths, callback, context);
#endif
  }

  monitor * monitor_factory::create_monitor(fsw_monitor_type type,
                                            vector<string> paths,
                                            FSW_EVENT_CALLBACK * callback,
                                            void * context)
  {
    switch (type)
    {
    case system_default_monitor_type:
      return create_default_monitor(paths, callback, context);

    case fsevents_monitor_type:
#if defined(HAVE_FSEVENTS_FILE_EVENTS)
      return new fsevents_monitor(paths, callback, context);
#else
      throw libfsw_exception("Unsupported monitor.", FSW_ERR_UNKNOWN_MONITOR_TYPE);
#endif

    case kqueue_monitor_type:
#if defined(HAVE_SYS_EVENT_H)
      return new kqueue_monitor(paths, callback, context);
#else
      throw libfsw_exception("Unsupported monitor.", FSW_ERR_UNKNOWN_MONITOR_TYPE);
#endif

    case inotify_monitor_type:
#if defined(HAVE_SYS_INOTIFY_H)
      return new inotify_monitor(paths, callback, context);
#else
      throw libfsw_exception("Unsupported monitor.", FSW_ERR_UNKNOWN_MONITOR_TYPE);
#endif

    case poll_monitor_type:
      return new poll_monitor(paths, callback, context);

    default:
      throw libfsw_exception("Unsupported monitor.", FSW_ERR_UNKNOWN_MONITOR_TYPE);
    }
  }

  void monitor::start()
  {
#ifdef HAVE_CXX_MUTEX
    lock_guard<mutex> run_guard(run_mutex);
#endif
    this->run();
  }

  vector<fsw_event_flag> monitor::filter_flags(const event &evt) const
  {
    // If there is nothing to filter, just return the original vector.
    if (event_type_filters.size() == 0) return evt.get_flags();
    
    vector<fsw_event_flag> filtered_flags;

    for (auto const & flag : evt.get_flags())
    {
      if (accept_event_type(flag)) filtered_flags.push_back(flag);
    }

    return filtered_flags;
  }

  void monitor::notify_events(const vector<event> &events) const
  {
    vector<event> filtered_events;

    for (auto const & event : events)
    {
      // Filter flags
      vector<fsw_event_flag> filtered_flags = filter_flags(event);
      if (filtered_flags.size() == 0) continue;

      if (!accept_path(event.get_path())) continue;

      filtered_events.push_back({event.get_path(), event.get_time(), filtered_flags});
    }

    if (filtered_events.size() > 0)
    {
      ostringstream log;
      log << _("Notifying events #: ") << filtered_events.size() << "\n";
      libfsw_log(log.str().c_str());

      callback(filtered_events, context);
    }
  }

  map<string, FSW_FN_MONITOR_CREATOR> & monitor_factory::creators_by_string()
  {
    static map<string, FSW_FN_MONITOR_CREATOR> creator_by_string_map;

    return creator_by_string_map;
  }

  monitor * monitor_factory::create_monitor(const string & name,
                                            vector<string> paths,
                                            FSW_EVENT_CALLBACK * callback,
                                            void * context)
  {
    auto i = creators_by_string().find(name);

    if (i == creators_by_string().end())
      return nullptr;
    else
      return i->second(paths, callback, context);
  }

  bool monitor_factory::exists_type(const string & name)
  {
    auto i = creators_by_string().find(name);

    return (i != creators_by_string().end());
  }

  void monitor_factory::register_creator(const string & name,
                                         FSW_FN_MONITOR_CREATOR creator)
  {
    creators_by_string()[name] = creator;
  }

  vector<string> monitor_factory::get_types()
  {
    vector<string> types;

    for (auto & i : creators_by_string())
    {
      types.push_back(i.first);
    }

    return types;
  }
}
