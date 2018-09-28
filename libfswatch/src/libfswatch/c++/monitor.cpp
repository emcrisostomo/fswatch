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
#include "monitor.hpp"
#include "monitor_factory.hpp"
#include "libfswatch_exception.hpp"
#include "../c/libfswatch_log.h"
#include "string/string_utils.hpp"
#include <cstdlib>
#include <memory>
#include <thread>
#include <regex>
#include <sstream>
#include <utility>
#include <ctime>

using namespace std::chrono;

namespace fsw
{
  struct compiled_monitor_filter
  {
    std::regex regex;
    fsw_filter_type type;
  };

#ifdef HAVE_CXX_MUTEX
  #define FSW_MONITOR_RUN_GUARD std::unique_lock<std::mutex> run_guard(run_mutex);
  #define FSW_MONITOR_RUN_GUARD_LOCK run_guard.lock();
  #define FSW_MONITOR_RUN_GUARD_UNLOCK run_guard.unlock();

  #define FSW_MONITOR_NOTIFY_GUARD std::unique_lock<std::mutex> notify_guard(notify_mutex);
#else
  #define FSW_MONITOR_RUN_GUARD
  #define FSW_MONITOR_RUN_GUARD_LOCK
  #define FSW_MONITOR_RUN_GUARD_UNLOCK

  #define FSW_MONITOR_NOTIFY_GUARD
#endif

  monitor::monitor(std::vector<std::string> paths,
                   FSW_EVENT_CALLBACK *callback,
                   void *context) :
    paths(std::move(paths)), callback(callback), context(context), latency(1)
  {
    if (callback == nullptr)
    {
      throw libfsw_exception(_("Callback cannot be null."),
                             FSW_ERR_CALLBACK_NOT_SET);
    }

#ifdef HAVE_INACTIVITY_CALLBACK
    milliseconds epoch =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    last_notification.store(epoch);
#endif
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

  void monitor::set_fire_idle_event(bool fire_idle_event)
  {
    this->fire_idle_event = fire_idle_event;
  }

  milliseconds monitor::get_latency_ms() const
  {
    return milliseconds((long long) (latency * 1000 * 1.1));
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

  void
  monitor::set_event_type_filters(const std::vector<fsw_event_type_filter>& filters)
  {
    event_type_filters.clear();

    for (const auto& filter : filters) add_event_type_filter(filter);
  }

  void monitor::add_filter(const monitor_filter& filter)
  {
    std::regex::flag_type regex_flags = std::regex::basic;

    if (filter.extended) regex_flags = std::regex::extended;
    if (!filter.case_sensitive) regex_flags |= std::regex::icase;

    try
    {
      this->filters.push_back({std::regex(filter.text, regex_flags),
                               filter.type});
    }
    catch (std::regex_error& error)
    {
      throw libfsw_exception(
        string_utils::string_from_format(
          _("An error occurred during the compilation of %s"),
          filter.text.c_str()),
        FSW_ERR_INVALID_REGEX);
    }
  }

  void monitor::set_property(const std::string& name, const std::string& value)
  {
    properties[name] = value;
  }

  void monitor::set_properties(const std::map<std::string, std::string> options)
  {
    properties = options;
  }

  std::string monitor::get_property(std::string name)
  {
    return properties[name];
  }

  void monitor::set_filters(const std::vector<monitor_filter>& filters)
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
    if (event_type_filters.empty()) return true;

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

  bool monitor::accept_path(const std::string& path) const
  {
    bool is_excluded = false;

    for (const auto& filter : filters)
    {
      if (std::regex_search(path, filter.regex))
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
    stop();
  }

#ifdef HAVE_INACTIVITY_CALLBACK

  void monitor::inactivity_callback(monitor *mon)
  {
    if (!mon) throw libfsw_exception(_("Callback argument cannot be null."));

    FSW_ELOG(_("Inactivity notification thread: starting\n"));

    for (;;)
    {
      std::unique_lock<std::mutex> run_guard(mon->run_mutex);
      if (mon->should_stop) break;
      run_guard.unlock();

      milliseconds elapsed =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch())
        - mon->last_notification.load();

      // Sleep and loop again if sufficient time has not elapsed yet.
      if (elapsed < mon->get_latency_ms())
      {
        milliseconds to_sleep = mon->get_latency_ms() - elapsed;
        seconds max_sleep_time(2);

        std::this_thread::sleep_for(
          to_sleep > max_sleep_time ? max_sleep_time : to_sleep);
        continue;
      }

      // Build a fake event.
      time_t curr_time;
      time(&curr_time);

      std::vector<event> events;
      events.push_back({"", curr_time, {NoOp}});

      mon->notify_events(events);
    }

    FSW_ELOG(_("Inactivity notification thread: exiting\n"));
  }

#endif

  void monitor::start()
  {
    FSW_MONITOR_RUN_GUARD;
    if (this->running) return;

    this->running = true;
    FSW_MONITOR_RUN_GUARD_UNLOCK;

    // Fire the inactivity thread
    std::unique_ptr<std::thread> inactivity_thread;
#ifdef HAVE_INACTIVITY_CALLBACK
    if (fire_idle_event)
      inactivity_thread.reset(
        new std::thread(monitor::inactivity_callback, this));
#endif

    // Fire the monitor run loop.
    this->run();

    // Join the inactivity thread and wait until it stops.
    FSW_ELOG(_("Inactivity notification thread: joining\n"));
    if (inactivity_thread) inactivity_thread->join();

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

    FSW_ELOG(_("Stopping the monitor.\n"));
    this->should_stop = true;
    on_stop();
  }

  bool monitor::is_running()
  {
    FSW_MONITOR_RUN_GUARD;
    return this->running;
  }

  std::vector<fsw_event_flag> monitor::filter_flags(const event& evt) const
  {
    // If there is nothing to filter, just return the original vector.
    if (event_type_filters.empty()) return evt.get_flags();

    std::vector<fsw_event_flag> filtered_flags;

    for (auto const& flag : evt.get_flags())
    {
      if (accept_event_type(flag)) filtered_flags.push_back(flag);
    }

    return filtered_flags;
  }

  void monitor::notify_overflow(const std::string& path) const
  {
    if (!allow_overflow) throw libfsw_exception(_("Event queue overflow."));

    time_t curr_time;
    time(&curr_time);

    notify_events({{path, curr_time, {fsw_event_flag::Overflow}}});
  }

  void monitor::notify_events(const std::vector<event>& events) const
  {
    FSW_MONITOR_NOTIFY_GUARD;

    // Update the last notification timestamp
#ifdef HAVE_INACTIVITY_CALLBACK
    milliseconds now =
      duration_cast<milliseconds>(
        system_clock::now().time_since_epoch());
    last_notification.store(now);
#endif

    std::vector<event> filtered_events;

    for (auto const& event : events)
    {
      // Filter flags
      std::vector<fsw_event_flag> filtered_flags = filter_flags(event);

      if (filtered_flags.empty()) continue;
      if (!accept_path(event.get_path())) continue;

      filtered_events.emplace_back(event.get_path(),
                                   event.get_time(),
                                   filtered_flags);
    }

    if (!filtered_events.empty())
    {
      FSW_ELOG(string_utils::string_from_format(_("Notifying events #: %d.\n"),
                                                filtered_events.size()).c_str());

      callback(filtered_events, context);
    }
  }

  void monitor::on_stop()
  {
    // No-op implementation.
  }
}
