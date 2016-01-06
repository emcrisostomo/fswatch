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
#include "string/string_utils.hpp"
#include <cstdlib>
#include <regex.h>
#include <sstream>
#include <time.h>

using namespace std;

namespace fsw
{
  struct compiled_monitor_filter
  {
    regex_t regex;
    fsw_filter_type type;
  };

#ifdef HAVE_CXX_MUTEX
  #define FSW_MONITOR_RUN_GUARD unique_lock<mutex> run_guard(run_mutex);
  #define FSW_MONITOR_RUN_GUARD_LOCK run_guard.lock();
  #define FSW_MONITOR_RUN_GUARD_UNLOCK run_guard.unlock();
#else
  #define FSW_MONITOR_RUN_GUARD
  #define FSW_MONITOR_RUN_GUARD_LOCK
  #define FSW_MONITOR_RUN_GUARD_UNLOCK
#endif

  monitor::monitor(vector<string> paths,
                   FSW_EVENT_CALLBACK *callback,
                   void *context) :
    paths(std::move(paths)), callback(callback), context(context)
  {
    if (callback == nullptr)
    {
      throw libfsw_exception(_("Callback cannot be null."),
                             FSW_ERR_CALLBACK_NOT_SET);
    }
  }

  void monitor::set_allow_overflow(bool overflow)
  {
    allow_overflow = overflow;
  }

  void monitor::set_latency(double latency)
  {
    if (latency < 0)
    {
      throw libfsw_exception(_("Latency cannot be negative."),
                             FSW_ERR_INVALID_LATENCY);
    }

    this->latency = latency;
  }

  void monitor::set_recursive(bool recursive)
  {
    this->recursive = recursive;
  }

  void monitor::set_directory_only(bool directory_only)
  {
    this->directory_only = directory_only;
  }

  void monitor::add_event_type_filter(const fsw_event_type_filter& filter)
  {
    this->event_type_filters.push_back(filter);
  }

  void monitor::set_event_type_filters(
    const vector<fsw_event_type_filter>& filters)
  {
    event_type_filters.clear();

    for (const auto& filter : filters) add_event_type_filter(filter);
  }

  void monitor::add_filter(const monitor_filter& filter)
  {
    regex_t regex;
    int flags = 0;

    if (!filter.case_sensitive) flags |= REG_ICASE;
    if (filter.extended) flags |= REG_EXTENDED;

    if (regcomp(&regex, filter.text.c_str(), flags))
    {
      throw libfsw_exception(
        string_utils::string_from_format(
          _("An error occurred during the compilation of %s"),
          filter.text.c_str()),
        FSW_ERR_INVALID_REGEX);
    }

    this->filters.push_back({regex, filter.type});
  }

  void monitor::set_property(const std::string& name, const std::string& value)
  {
    properties[name] = value;
  }

  void monitor::set_properties(const map<string, string>& options)
  {
    properties = options;
  }

  string monitor::get_property(string name)
  {
    return properties[name];
  }

  void monitor::set_filters(const vector<monitor_filter>& filters)
  {
    for (const monitor_filter& filter : filters)
    {
      add_filter(filter);
    }
  }

  void monitor::set_follow_symlinks(bool follow)
  {
    follow_symlinks = follow;
  }

  void monitor::set_watch_access(bool access)
  {
    watch_access = access;
  }

  bool monitor::accept_event_type(fsw_event_flag event_type) const
  {
    // If no filters are set, then accept the event.
    if (event_type_filters.size() == 0) return true;

    // If filters are set, accept the event only if present amongst the filters.
    for (const auto& filter : event_type_filters)
    {
      if (filter.flag == event_type)
      {
        return true;
      }
    }

    // If no filters match, then reject the event.
    return false;
  }

  bool monitor::accept_path(const string& path) const
  {
    return accept_path(path.c_str());
  }

  bool monitor::accept_path(const char *path) const
  {
    bool is_excluded = false;

    for (const auto& filter : filters)
    {
      if (regexec(&filter.regex, path, 0, nullptr, 0) == 0)
      {
        if (filter.type == fsw_filter_type::filter_include) return true;

        is_excluded = (filter.type == fsw_filter_type::filter_exclude);
      }
    }

    return !is_excluded;
  }

  void *monitor::get_context() const
  {
    return context;
  }

  void monitor::set_context(void *context)
  {
    this->context = context;
  }

  monitor::~monitor()
  {
    for (auto& re : filters) regfree(&re.regex);

    filters.clear();
  }

  static monitor *create_default_monitor(vector<string> paths,
                                         FSW_EVENT_CALLBACK *callback,
                                         void *context)
  {
    fsw_monitor_type type;

#if defined(HAVE_FSEVENTS_FILE_EVENTS)
    type = fsw_monitor_type::fsevents_monitor_type;
#elif defined(HAVE_SYS_EVENT_H)
    type = fsw_monitor_type::kqueue_monitor_type;
#elif defined(HAVE_PORT_H)
    type = fsw_monitor_type::fen_monitor_type;
#elif defined(HAVE_SYS_INOTIFY_H)
    type = fsw_monitor_type::inotify_monitor_type;
#elif defined(HAVE_WINDOWS)
    type = fsw_monitor_type::windows_monitor_type;
#else
    type = fsw_monitor_type::poll_monitor_type;
#endif

    return monitor_factory::create_monitor(type, paths, callback, context);
  }

  monitor *monitor_factory::create_monitor(fsw_monitor_type type,
                                           vector<string> paths,
                                           FSW_EVENT_CALLBACK *callback,
                                           void *context)
  {
    switch (type)
    {
    case system_default_monitor_type:
      return create_default_monitor(paths, callback, context);

    default:
      auto c = creators_by_type().find(type);

      if (c == creators_by_type().end())
        throw libfsw_exception("Unsupported monitor.",
                               FSW_ERR_UNKNOWN_MONITOR_TYPE);
      return c->second(paths, callback, context);
    }
  }

  void monitor::start()
  {
    FSW_MONITOR_RUN_GUARD;
    if (this->running) return;

    this->running = true;
    FSW_MONITOR_RUN_GUARD_UNLOCK;

    this->run();

    FSW_MONITOR_RUN_GUARD_LOCK;
    this->running = false;
    this->should_stop = false;
    FSW_MONITOR_RUN_GUARD_UNLOCK;
  }

  void monitor::stop()
  {
    // Stopping a monitor is a cooperative task: the caller request a task to
    // stop and it's responsibility of each monitor to check for this flag and
    // timely stop the processing loop.
    FSW_MONITOR_RUN_GUARD;
    if (!this->running || this->should_stop) return;

    this->should_stop = true;
    on_stop();
  }

  bool monitor::is_running()
  {
    FSW_MONITOR_RUN_GUARD;
    return this->running;
  }

  vector<fsw_event_flag> monitor::filter_flags(const event& evt) const
  {
    // If there is nothing to filter, just return the original vector.
    if (event_type_filters.size() == 0) return evt.get_flags();

    vector<fsw_event_flag> filtered_flags;

    for (auto const& flag : evt.get_flags())
    {
      if (accept_event_type(flag)) filtered_flags.push_back(flag);
    }

    return filtered_flags;
  }

  void monitor::notify_overflow(const string& path) const
  {
    if (!allow_overflow) throw libfsw_exception(_("Event queue overflow."));

    time_t curr_time;
    time(&curr_time);

    notify_events({{path, curr_time, {fsw_event_flag::Overflow}}});
  }

  void monitor::notify_events(const vector<event>& events) const
  {
    vector<event> filtered_events;

    for (auto const& event : events)
    {
      // Filter flags
      vector<fsw_event_flag> filtered_flags = filter_flags(event);
      if (filtered_flags.size() == 0) continue;

      if (!accept_path(event.get_path())) continue;

      filtered_events.push_back(
        {event.get_path(), event.get_time(), filtered_flags});
    }

    if (filtered_events.size() > 0)
    {
      FSW_ELOG(string_utils::string_from_format(_("Notifying events #: %d.\n"),
                                                filtered_events.size()).c_str());

      callback(filtered_events, context);
    }
  }

  map<string, FSW_FN_MONITOR_CREATOR>& monitor_factory::creators_by_string()
  {
    static map<string, FSW_FN_MONITOR_CREATOR> creator_by_string_map;

    return creator_by_string_map;
  }

  map<fsw_monitor_type, FSW_FN_MONITOR_CREATOR>& monitor_factory::creators_by_type()
  {
    static map<fsw_monitor_type, FSW_FN_MONITOR_CREATOR> creator_by_type_map;

    return creator_by_type_map;
  }

  monitor *monitor_factory::create_monitor(const string& name,
                                           vector<string> paths,
                                           FSW_EVENT_CALLBACK *callback,
                                           void *context)
  {
    auto i = creators_by_string().find(name);

    if (i == creators_by_string().end())
      return nullptr;

    return i->second(paths, callback, context);
  }

  bool monitor_factory::exists_type(const string& name)
  {
    auto i = creators_by_string().find(name);

    return (i != creators_by_string().end());
  }

  bool monitor_factory::exists_type(const fsw_monitor_type& name)
  {
    auto i = creators_by_type().find(name);

    return (i != creators_by_type().end());
  }

  void monitor_factory::register_creator(const string& name,
                                         FSW_FN_MONITOR_CREATOR creator)
  {
    creators_by_string()[name] = creator;
  }

  void monitor_factory::register_creator_by_type(const fsw_monitor_type& type,
                                                 FSW_FN_MONITOR_CREATOR creator)
  {
    creators_by_type()[type] = creator;
  }


  vector<string> monitor_factory::get_types()
  {
    vector<string> types;

    for (const auto& i : creators_by_string())
    {
      types.push_back(i.first);
    }

    return types;
  }

  void monitor::on_stop()
  {
    // No-op implementation.
  }
}
