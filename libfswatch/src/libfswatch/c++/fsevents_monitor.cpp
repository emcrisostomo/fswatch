/*
 * Copyright (c) 2014-2022 Enrico M. Crisostomo
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
#include "libfswatch/libfswatch_config.h"
#include <memory>
#include <unistd.h> // isatty()
#include <cstdio> // fileno()
#include <thread>
#include "fsevents_monitor.hpp"
#include "libfswatch/gettext_defs.h"
#include "libfswatch_exception.hpp"
#include "libfswatch/c/libfswatch_log.h"
#  ifdef HAVE_CXX_MUTEX
#    include <mutex>
#  endif

namespace fsw
{
  using std::vector;
  using std::string;

  using FSEventFlagType =
    struct FSEventFlagType
    {
      FSEventStreamEventFlags flag;
      fsw_event_flag type;
    };

  static vector<FSEventFlagType> create_flag_type_vector()
  {
    vector<FSEventFlagType> flags;
#ifdef HAVE_MACOS_GE_10_5
    flags.push_back({kFSEventStreamEventFlagNone, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagMustScanSubDirs, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagUserDropped, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagKernelDropped, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagEventIdsWrapped, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagHistoryDone, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagRootChanged, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagMount, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagUnmount, fsw_event_flag::PlatformSpecific});
#endif

#ifdef HAVE_MACOS_GE_10_7
    flags.push_back({kFSEventStreamEventFlagItemChangeOwner, fsw_event_flag::OwnerModified});
    flags.push_back({kFSEventStreamEventFlagItemCreated, fsw_event_flag::Created});
    flags.push_back({kFSEventStreamEventFlagItemFinderInfoMod, fsw_event_flag::PlatformSpecific});
    flags.push_back({kFSEventStreamEventFlagItemFinderInfoMod, fsw_event_flag::AttributeModified});
    flags.push_back({kFSEventStreamEventFlagItemInodeMetaMod, fsw_event_flag::AttributeModified});
    flags.push_back({kFSEventStreamEventFlagItemIsDir, fsw_event_flag::IsDir});
    flags.push_back({kFSEventStreamEventFlagItemIsFile, fsw_event_flag::IsFile});
    flags.push_back({kFSEventStreamEventFlagItemIsSymlink, fsw_event_flag::IsSymLink});
    flags.push_back({kFSEventStreamEventFlagItemModified, fsw_event_flag::Updated});
    flags.push_back({kFSEventStreamEventFlagItemRemoved, fsw_event_flag::Removed});
    flags.push_back({kFSEventStreamEventFlagItemRenamed, fsw_event_flag::Renamed});
    flags.push_back({kFSEventStreamEventFlagItemXattrMod, fsw_event_flag::AttributeModified});
#endif

#ifdef HAVE_MACOS_GE_10_9
    flags.push_back({kFSEventStreamEventFlagOwnEvent, fsw_event_flag::AttributeModified});
#endif

#ifdef HAVE_MACOS_GE_10_10
    flags.push_back({kFSEventStreamEventFlagItemIsHardlink, fsw_event_flag::Link});
    flags.push_back({kFSEventStreamEventFlagItemIsLastHardlink, fsw_event_flag::Link});
    flags.push_back({kFSEventStreamEventFlagItemIsLastHardlink, fsw_event_flag::PlatformSpecific});
#endif

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

    create_stream(pathsToWatch);

    if (!stream)
      throw libfsw_exception(_("Event stream could not be created."));

    // Creating dispatch queue
    fsevents_queue = dispatch_queue_create("fswatch_event_queue", NULL);
    FSEventStreamSetDispatchQueue(stream, fsevents_queue);

    FSW_ELOG(_("Starting event stream...\n"));
    FSEventStreamStart(stream);

#ifdef HAVE_CXX_MUTEX
    run_loop_lock.unlock();
#endif

    for(;;)
    {
#ifdef HAVE_CXX_MUTEX
      run_loop_lock.lock();
      if (should_stop) break;
      run_loop_lock.unlock();
#endif

      std::this_thread::sleep_for(std::chrono::milliseconds((long long) (latency * 1000)));
    }

    // Deinitialization part
    FSW_ELOG(_("Stopping event stream...\n"));
    FSEventStreamStop(stream);

    FSW_ELOG(_("Invalidating event stream...\n"));
    FSEventStreamInvalidate(stream);

    FSW_ELOG(_("Releasing event stream...\n"));
    FSEventStreamRelease(stream);

    dispatch_release(fsevents_queue);
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

  void fsevents_monitor::fsevents_callback(ConstFSEventStreamRef,
                                           void *clientCallBackInfo,
                                           size_t numEvents,
                                           void *eventPaths,
                                           const FSEventStreamEventFlags eventFlags[],
                                           const FSEventStreamEventId *)
  {
    const auto *fse_monitor = static_cast<fsevents_monitor *> (clientCallBackInfo);

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

    if (no_defer.empty())
      return (!isatty(fileno(stdin)));

    return (no_defer == "true");
  }

  void fsevents_monitor::create_stream(CFArrayRef pathsToWatch)
  {
    std::unique_ptr<FSEventStreamContext> context(new FSEventStreamContext());
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
                                 context.get(),
                                 pathsToWatch,
                                 kFSEventStreamEventIdSinceNow,
                                 latency,
                                 streamFlags);
  }
}
