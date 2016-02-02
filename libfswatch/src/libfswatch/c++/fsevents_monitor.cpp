/*
 * Copyright (c) 2014-2016 Enrico M. Crisostomo
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

#include "fsevents_monitor.hpp"
#include "gettext_defs.h"
#include "libfswatch_exception.hpp"
#include "c/libfswatch_log.h"
#include <memory>
#include <thread>

using namespace std;
using namespace std::chrono;

namespace fsw
{
  typedef struct FSEventFlagType
  {
    FSEventStreamEventFlags flag;
    fsw_event_flag type;
  } FSEventFlagType;

  static vector<FSEventFlagType> create_flag_type_vector()
  {
    vector<FSEventFlagType> flags;
    flags.push_back({kFSEventStreamEventFlagNone, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagMustScanSubDirs, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagUserDropped, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagKernelDropped, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagEventIdsWrapped, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagHistoryDone, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagRootChanged, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagMount, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagUnmount, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagItemCreated, fsw_event_flag::Created});
    flags.push_back({kFSEventStreamEventFlagItemRemoved, fsw_event_flag::Removed});
    flags.push_back({kFSEventStreamEventFlagItemInodeMetaMod, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagItemRenamed, fsw_event_flag::Renamed});
    flags.push_back({kFSEventStreamEventFlagItemModified, fsw_event_flag::Updated});
    flags.push_back({kFSEventStreamEventFlagItemFinderInfoMod, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagItemChangeOwner, fsw_event_flag::OwnerModified});
    flags.push_back({kFSEventStreamEventFlagItemXattrMod, fsw_event_flag::AttributeModified});
    flags.push_back({kFSEventStreamEventFlagItemIsFile, fsw_event_flag::IsFile});
    flags.push_back({kFSEventStreamEventFlagItemIsDir, fsw_event_flag::IsDir});
    flags.push_back({kFSEventStreamEventFlagItemIsSymlink, fsw_event_flag::IsSymLink});

    return flags;
  }

  static const vector<FSEventFlagType> event_flag_type = create_flag_type_vector();

  REGISTER_MONITOR_IMPL(fsevents_monitor, fsevents_monitor_type);

  fsevents_monitor::fsevents_monitor(vector<string> paths_to_monitor,
                                     FSW_EVENT_CALLBACK *callback,
                                     void *context) :
    monitor(paths_to_monitor, callback, context),
    last_notification(duration_cast<milliseconds>(system_clock::now().time_since_epoch()))
  {
  }

  fsevents_monitor::~fsevents_monitor()
  {
  }

  void fsevents_monitor::run()
  {
#ifdef HAVE_CXX_MUTEX
    unique_lock<mutex> run_loop_lock(run_mutex);
#endif
    if (stream) return;

    // parsing paths
    vector<CFStringRef> dirs;

    for (string path : paths)
    {
      dirs.push_back(CFStringCreateWithCString(NULL,
                                               path.c_str(),
                                               kCFStringEncodingUTF8));
    }

    if (dirs.size() == 0) return;

    CFArrayRef pathsToWatch =
      CFArrayCreate(NULL,
                    reinterpret_cast<const void **> (&dirs[0]),
                    dirs.size(),
                    &kCFTypeArrayCallBacks);

    FSEventStreamContext *context = new FSEventStreamContext();
    context->version = 0;
    context->info = this;
    context->retain = nullptr;
    context->release = nullptr;
    context->copyDescription = nullptr;

    FSW_ELOG(_("Creating FSEvent stream...\n"));
    stream = FSEventStreamCreate(NULL,
                                 &fsevents_monitor::fsevents_callback,
                                 context,
                                 pathsToWatch,
                                 kFSEventStreamEventIdSinceNow,
                                 latency,
                                 kFSEventStreamCreateFlagFileEvents);

    if (!stream)
      throw libfsw_exception(_("Event stream could not be created."));

    // Fire the inactivity thread
    std::unique_ptr<std::thread> inactivity_thread;
#ifdef HAVE_CXX_MUTEX
    inactivity_thread.reset(new std::thread(fsevents_monitor::inactivity_callback, this));
#endif

    // Fire the event loop
    run_loop = CFRunLoopGetCurrent();
    run_loop_lock.unlock();

    FSW_ELOG(_("Scheduling stream with run loop...\n"));
    FSEventStreamScheduleWithRunLoop(stream,
                                     run_loop,
                                     kCFRunLoopDefaultMode);

    FSW_ELOG(_("Starting event stream...\n"));
    FSEventStreamStart(stream);

    FSW_ELOG(_("Starting run loop...\n"));
    CFRunLoopRun();

    // Join the inactivity thread and wait until it stops.
    FSW_ELOG(_("Inactivity notification thread: joining\n"));
    if (inactivity_thread) inactivity_thread->join();
  }

  /*
   * on_stop() is designed to be invoked with a lock on the run_mutex.
   */
  void fsevents_monitor::on_stop()
  {
    if (!run_loop) throw libfsw_exception(_("run loop is null"));

    FSW_ELOG(_("Stopping run loop...\n"));
    CFRunLoopStop(run_loop);

    run_loop = nullptr;

    FSW_ELOG(_("Stopping event stream...\n"));
    FSEventStreamStop(stream);

    FSW_ELOG(_("Invalidating event stream...\n"));
    FSEventStreamInvalidate(stream);

    FSW_ELOG(_("Releasing event stream...\n"));
    FSEventStreamRelease(stream);

    stream = nullptr;
  }

  static vector<fsw_event_flag> decode_flags(FSEventStreamEventFlags flag)
  {
    vector<fsw_event_flag> evt_flags;

    for (const FSEventFlagType& type : event_flag_type)
    {
      if (flag & type.flag)
      {
        evt_flags.push_back(type.type);
      }
    }

    return evt_flags;
  }

  void fsevents_monitor::notify_events_sync(const std::vector<event>& events) const
  {
#ifdef HAVE_CXX_MUTEX
    lock_guard<mutex> notify_lock(notify_mutex);
#endif
    notify_events(events);
  }

#ifdef HAVE_CXX_MUTEX
  void fsevents_monitor::inactivity_callback(fsevents_monitor *fse_monitor)
  {
    if (!fse_monitor)
    {
      throw libfsw_exception(_("Callback argument cannot be null."));
    }

    FSW_ELOG(_("Inactivity notification thread: starting\n"));

    for (;;)
    {
      std::this_thread::sleep_for(nanoseconds((long)(fse_monitor->latency * 1000 * 1000 * 1000)));

      unique_lock<mutex> run_guard(fse_monitor->run_mutex);
      if (fse_monitor->should_stop) break;
      run_guard.unlock();

      timeout_callback(fse_monitor);
    }

    FSW_ELOG(_("Inactivity notification thread: exiting\n"));
  }

  void fsevents_monitor::timeout_callback(fsevents_monitor *fse_monitor)
  {
    milliseconds now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds previous = fse_monitor->last_notification.load(memory_order_acquire);

    // If the distance between the timeout event and the last notified event is
    // greater than the latency, then do nothing.
    if ((now - previous) < milliseconds((long) (fse_monitor->latency * 1000)))
      return;

    fse_monitor->last_notification.compare_exchange_weak(previous, now, memory_order_acq_rel);

    // Build a fake
    time_t curr_time;
    time(&curr_time);

    vector<event> events;
    events.push_back({"", curr_time, {NoOp}});

    fse_monitor->notify_events_sync(events);
  }
#endif

  void fsevents_monitor::fsevents_callback(ConstFSEventStreamRef streamRef,
                                           void *clientCallBackInfo,
                                           size_t numEvents,
                                           void *eventPaths,
                                           const FSEventStreamEventFlags eventFlags[],
                                           const FSEventStreamEventId eventIds[])
  {
    fsevents_monitor *fse_monitor =
      static_cast<fsevents_monitor *> (clientCallBackInfo);

    if (!fse_monitor)
    {
      throw libfsw_exception(_("The callback info cannot be cast to fsevents_monitor."));
    }

    // Build the notification objects.
    vector<event> events;

    time_t curr_time;
    time(&curr_time);

    for (size_t i = 0; i < numEvents; ++i)
    {
      events.push_back({((char **) eventPaths)[i], curr_time, decode_flags(eventFlags[i])});
    }

    if (events.size() > 0)
    {
      // Update the last notification timestamp
      milliseconds now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
      fse_monitor->last_notification.exchange(now, memory_order_release);

      fse_monitor->notify_events_sync(events);
    }
  }
}
