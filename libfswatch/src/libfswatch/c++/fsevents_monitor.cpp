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
#  ifdef HAVE_CXX_MUTEX
#    include <mutex>
#  endif

namespace fsw
{
  using std::vector;
  using std::string;
  
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

  fsevents_monitor::fsevents_monitor(vector<string> paths_to_monitor,
                                     FSW_EVENT_CALLBACK *callback,
                                     void *context) :
    monitor(std::move(paths_to_monitor), callback, context)
  {
  }


  void fsevents_monitor::run()
  {
#ifdef HAVE_CXX_MUTEX
    std::unique_lock<std::mutex> run_loop_lock(run_mutex);
#endif

    if (stream) return;

    // parsing paths
    vector<CFStringRef> dirs;

    for (const string& path : paths)
    {
      dirs.push_back(CFStringCreateWithCString(nullptr,
                                               path.c_str(),
                                               kCFStringEncodingUTF8));
    }

    if (dirs.empty()) return;

    CFArrayRef pathsToWatch =
      CFArrayCreate(nullptr,
                    reinterpret_cast<const void **> (&dirs[0]),
                    dirs.size(),
                    &kCFTypeArrayCallBacks);

    auto *context = new FSEventStreamContext();
    context->version = 0;
    context->info = this;
    context->retain = nullptr;
    context->release = nullptr;
    context->copyDescription = nullptr;

    FSEventStreamCreateFlags streamFlags = kFSEventStreamCreateFlagFileEvents;
    if (this->no_defer()) streamFlags |= kFSEventStreamCreateFlagNoDefer;

    FSW_ELOG(_("Creating FSEvent stream...\n"));
    stream = FSEventStreamCreate(nullptr,
                                 &fsevents_monitor::fsevents_callback,
                                 context,
                                 pathsToWatch,
                                 kFSEventStreamEventIdSinceNow,
                                 latency,
                                 streamFlags);

    delete context;

    if (!stream)
      throw libfsw_exception(_("Event stream could not be created."));

    // Fire the event loop
    run_loop = CFRunLoopGetCurrent();

    // Loop Initialization

    FSW_ELOG(_("Scheduling stream with run loop...\n"));
    FSEventStreamScheduleWithRunLoop(stream,
                                     run_loop,
                                     kCFRunLoopDefaultMode);

    FSW_ELOG(_("Starting event stream...\n"));
    FSEventStreamStart(stream);

#ifdef HAVE_CXX_MUTEX
    run_loop_lock.unlock();
#endif

    // Loop

    FSW_ELOG(_("Starting run loop...\n"));
    CFRunLoopRun();

    // Deinitialization part

    FSW_ELOG(_("Stopping event stream...\n"));
    FSEventStreamStop(stream);

    FSW_ELOG(_("Invalidating event stream...\n"));
    FSEventStreamInvalidate(stream);

    FSW_ELOG(_("Releasing event stream...\n"));
    FSEventStreamRelease(stream);

    stream = nullptr;
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

  void fsevents_monitor::fsevents_callback(ConstFSEventStreamRef streamRef,
                                           void *clientCallBackInfo,
                                           size_t numEvents,
                                           void *eventPaths,
                                           const FSEventStreamEventFlags eventFlags[],
                                           const FSEventStreamEventId eventIds[])
  {
    auto *fse_monitor = static_cast<fsevents_monitor *> (clientCallBackInfo);

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
      events.emplace_back(((char **) eventPaths)[i],
                          curr_time,
                          decode_flags(eventFlags[i]));
    }

    if (!events.empty())
    {
      fse_monitor->notify_events(events);
    }
  }

  bool fsevents_monitor::no_defer()
  {
    string no_defer = get_property(DARWIN_EVENTSTREAM_NO_DEFER);

    return (no_defer == "true");
  }
}
