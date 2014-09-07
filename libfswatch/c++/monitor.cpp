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
#include "monitor.h"
#include "libfswatch_exception.h"
#include <cstdlib>
#ifdef HAVE_REGCOMP
#  include <regex.h>
#endif

/*
 * Conditionally include monitor headers for default construction.
 */
#if defined(HAVE_FSEVENTS_FILE_EVENTS)
#  include "fsevents_monitor.h"
#endif
#if defined(HAVE_SYS_EVENT_H)
#  include "kqueue_monitor.h"
#endif
#if defined(HAVE_SYS_INOTIFY_H)
#  include "inotify_monitor.h"
#endif
#include "poll_monitor.h"

#include <iostream>

using namespace std;

namespace fsw
{

  struct compiled_monitor_filter
  {
#ifdef HAVE_REGCOMP    
    regex_t regex;
    fsw_filter_type type;
#endif
  };

  monitor::monitor(std::vector<std::string> paths,
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

  void monitor::set_filters(const std::vector<monitor_filter> &filters)
  {
#ifdef HAVE_REGCOMP
    for (const monitor_filter &filter : filters)
    {
      add_filter(filter);
    }
#endif
  }

  void monitor::set_follow_symlinks(bool follow)
  {
    follow_symlinks = follow;
  }

  bool monitor::accept_path(const string &path)
  {
    return accept_path(path.c_str());
  }

  bool monitor::accept_path(const char *path)
  {
#ifdef HAVE_REGCOMP
    for (auto &filter : filters)
    {
      if (::regexec(&filter.regex, path, 0, nullptr, 0) == 0)
      {
        return filter.type == fsw_filter_type::filter_include;
      }
    }
#endif

    return true;
  }

  void * monitor::get_context()
  {
    return context;
  }

  void monitor::set_context(void * context)
  {
    this->context = context;
  }

  monitor::~monitor()
  {
#ifdef HAVE_REGCOMP
    for (auto &re : filters)
    {
      ::regfree(&re.regex);
    }

    filters.clear();
#endif
  }

  monitor * monitor::create_default_monitor(std::vector<std::string> paths,
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

  monitor * monitor::create_monitor(fsw_monitor_type type,
                                    std::vector<std::string> paths,
                                    FSW_EVENT_CALLBACK * callback,
                                    void * context)
  {
    switch (type)
    {
    case system_default_monitor_type:
      return monitor::create_default_monitor(paths, callback, context);

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

  map<string, fsw_monitor_type> & monitor_factory::type_by_string()
  {
    static map<string, fsw_monitor_type> type_by_string_map;

    return type_by_string_map;
  };

  monitor * monitor_factory::create_monitor_by_name(const std::string& name,
                                                    std::vector<std::string> paths,
                                                    FSW_EVENT_CALLBACK * callback,
                                                    void * context)
  {
    auto i = type_by_string().find(name);

    if (i == type_by_string().end())
      return nullptr;
    else
      return monitor::create_monitor(i->second, paths, callback, context);
  }

  bool monitor_factory::exists_type(const std::string& name)
  {
    auto i = type_by_string().find(name);

    return (i != type_by_string().end());
  }

  void monitor_factory::register_type(const std::string& name, fsw_monitor_type type)
  {
    type_by_string()[name] = type;
  }

  vector<string> monitor_factory::get_types()
  {
    vector<string> types;

    for (auto & i : type_by_string())
    {
      types.push_back(i.first);
    }

    return types;
  }

  monitor_registrant::monitor_registrant(const std::string & name, fsw_monitor_type type)
  {
    monitor_factory::register_type(name, type);
  }
}
