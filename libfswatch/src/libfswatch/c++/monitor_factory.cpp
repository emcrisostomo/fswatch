#include <utility>

/*
 * Copyright (c) 2014-2018 Enrico M. Crisostomo
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
#include "monitor_factory.hpp"
#include "libfswatch_exception.hpp"
#if defined(HAVE_FSEVENTS_FILE_EVENTS)
  #include "fsevents_monitor.hpp"
#endif
#if defined(HAVE_SYS_EVENT_H)
  #include "kqueue_monitor.hpp"
#endif
#if defined(HAVE_PORT_H)
  #include "fen_monitor.hpp"
#endif
#if defined(HAVE_SYS_INOTIFY_H)
  #include "inotify_monitor.hpp"
#endif
#if defined(HAVE_WINDOWS)
  #include "windows_monitor.hpp"
#endif
#include "poll_monitor.hpp"

namespace fsw
{
  static monitor *create_default_monitor(std::vector<std::string> paths,
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

    return monitor_factory::create_monitor(type,
                                           std::move(paths),
                                           callback,
                                           context);
  }

  monitor *monitor_factory::create_monitor(fsw_monitor_type type,
                                           std::vector<std::string> paths,
                                           FSW_EVENT_CALLBACK *callback,
                                           void *context)
  {
    switch (type)
    {
    case system_default_monitor_type:
      return create_default_monitor(paths, callback, context);

    default:
#if defined(HAVE_FSEVENTS_FILE_EVENTS)
      return new fsevents_monitor(paths, callback, context);
#elif defined(HAVE_SYS_EVENT_H)
      return new kqueue_monitor(paths, callback, context);
#elif defined(HAVE_PORT_H)
      return new fen_monitor(paths, callback, context);
#elif defined(HAVE_SYS_INOTIFY_H)
      return new inotify_monitor(paths, callback, context);
#elif defined(HAVE_WINDOWS)
      return new windows_monitor(paths, callback, context);
#else
      return new poll_monitor(paths, callback, context);
#endif
    }
  }

  std::map<std::string, fsw_monitor_type>& monitor_factory::creators_by_string()
  {
#define fsw_quote(x) #x
    static std::map<std::string, fsw_monitor_type> creator_by_string_set;

#if defined(HAVE_FSEVENTS_FILE_EVENTS)
    creator_by_string_set[fsw_quote(fsevents_monitor)] = fsw_monitor_type::fsevents_monitor_type;
#endif
#if defined(HAVE_SYS_EVENT_H)
    creator_by_string_set[fsw_quote(kqueue_monitor)] = fsw_monitor_type::kqueue_monitor_type;
#endif
#if defined(HAVE_PORT_H)
    creator_by_string_set[fsw_quote(fen_monitor)] = fsw_monitor_type::fen_monitor_type;
#endif
#if defined(HAVE_SYS_INOTIFY_H)
    creator_by_string_set[fsw_quote(inotify_monitor)] = fsw_monitor_type::inotify_monitor_type;
#endif
#if defined(HAVE_WINDOWS)
    creator_by_string_set[fsw_quote(windows_monitor)] = fsw_monitor_type::windows_monitor_type;
#endif
    creator_by_string_set[fsw_quote(poll_monitor)] = fsw_monitor_type::poll_monitor_type;

    return creator_by_string_set;
#undef fsw_quote
  }

  monitor *monitor_factory::create_monitor(const std::string& name,
                                           std::vector<std::string> paths,
                                           FSW_EVENT_CALLBACK *callback,
                                           void *context)
  {
    auto i = creators_by_string().find(name);

    if (i == creators_by_string().end())
      return nullptr;

    return create_monitor(i->second, std::move(paths), callback, context);
  }

  bool monitor_factory::exists_type(const std::string& name)
  {
    auto i = creators_by_string().find(name);

    return (i != creators_by_string().end());
  }

  std::vector<std::string> monitor_factory::get_types()
  {
    std::vector<std::string> types;

    for (const auto& i : creators_by_string())
    {
      types.push_back(i.first);
    }

    return types;
  }
}