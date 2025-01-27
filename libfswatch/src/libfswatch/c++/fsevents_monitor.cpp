/*
 * Copyright (c) 2014-2025 Enrico M. Crisostomo
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
#include <libfswatch/libfswatch_config.h>
#include <memory>
#include <unistd.h> // isatty()
#include <cstdio> // fileno()
#include <thread>
#include "fsevents_monitor.hpp"
#include "libfswatch/gettext_defs.h"
#include "libfswatch_exception.hpp"
#include "libfswatch/c/libfswatch_log.h"
#include <mutex>

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

  class fsevents_monitor::Impl
  {
  public:
    Impl() = default;
    ~Impl() = default;
  
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    FSEventStreamRef stream = nullptr;
#ifdef HAVE_MACOS_GE_10_6
    dispatch_queue_t fsevents_queue = nullptr;
#else
    CFRunLoopRef run_loop = nullptr;
#endif
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

#ifdef HAVE_MACOS_GE_10_13
    flags.push_back({kFSEventStreamEventFlagItemCloned, fsw_event_flag::PlatformSpecific});
#endif

    return flags;
  }

  static const vector<FSEventFlagType> event_flag_type = create_flag_type_vector();

  fsevents_monitor::fsevents_monitor(vector<string> paths_to_monitor,
                                     FSW_EVENT_CALLBACK *callback,
                                     void *context) :
    monitor(std::move(paths_to_monitor), callback, context), pImpl(std::make_unique<Impl>())
  {
  }

  void fsevents_monitor::run()
  {
    std::unique_lock<std::mutex> run_loop_lock(run_mutex);

    if (pImpl->stream) return;

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

    if (!pImpl->stream)
      throw libfsw_exception(_("Event stream could not be created."));

#ifdef HAVE_MACOS_GE_10_6
    // Creating dispatch queue
    pImpl->fsevents_queue = dispatch_queue_create("fswatch_event_queue", nullptr);
    FSEventStreamSetDispatchQueue(pImpl->stream, pImpl->fsevents_queue);
#else
    // Fire the event loop
    pImpl->run_loop = CFRunLoopGetCurrent();

    // Loop Initialization
    FSW_ELOG(_("Scheduling stream with run loop...\n"));
    FSEventStreamScheduleWithRunLoop(pImpl->stream,
                                     pImpl->run_loop,
                                     kCFRunLoopDefaultMode);
#endif

    FSW_ELOG(_("Starting event stream...\n"));
    FSEventStreamStart(pImpl->stream);

    run_loop_lock.unlock();

#ifdef HAVE_MACOS_GE_10_6
    for(;;)
    {
      run_loop_lock.lock();
      if (should_stop) break;
      run_loop_lock.unlock();

      std::this_thread::sleep_for(std::chrono::milliseconds((long long) (latency * 1000)));
    }
#else
    // Loop
    FSW_ELOG(_("Starting run loop...\n"));
    CFRunLoopRun();
#endif

    // Deinitialization part
    FSW_ELOG(_("Stopping event stream...\n"));
    FSEventStreamStop(pImpl->stream);

    FSW_ELOG(_("Invalidating event stream...\n"));
    FSEventStreamInvalidate(pImpl->stream);

    FSW_ELOG(_("Releasing event stream...\n"));
    FSEventStreamRelease(pImpl->stream);

#ifdef HAVE_MACOS_GE_10_6
    dispatch_release(pImpl->fsevents_queue);
#endif
    pImpl->stream = nullptr;
  }

  /*
   * on_stop() is designed to be invoked with a lock on the run_mutex.
   */
  void fsevents_monitor::on_stop()
  {
#ifndef HAVE_MACOS_GE_10_6
    if (!pImpl->run_loop) throw libfsw_exception(_("run loop is null"));

    FSW_ELOG(_("Stopping run loop...\n"));
    CFRunLoopStop(pImpl->run_loop);

    pImpl->run_loop = nullptr;
#endif
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
#ifdef HAVE_MACOS_GE_10_13
      auto path_info_dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex((CFArrayRef) eventPaths,
                                                                                i));
      auto path = static_cast<CFStringRef>(CFDictionaryGetValue(path_info_dict,
                                                                kFSEventStreamEventExtendedDataPathKey));
      auto cf_inode = static_cast<CFNumberRef>(CFDictionaryGetValue(path_info_dict,
                                                                    kFSEventStreamEventExtendedFileIDKey));

      // Get the length of the UTF8-encoded CFString in bytes
      CFIndex length = CFStringGetLength(path);
      CFIndex max_path_size = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;

      // Allocate a buffer dynamically
      std::vector<char> path_buffer(max_path_size);
      if (!CFStringGetCString(path, path_buffer.data(), max_path_size, kCFStringEncodingUTF8))
      {
          std::cerr << "Warning: Failed to convert CFStringRef to C string." << std::endl;
          continue;
      }

      unsigned long inode;
      CFNumberGetValue(cf_inode, kCFNumberLongType, &inode);
      events.emplace_back(std::string(path_buffer.data()),
                          curr_time,
                          decode_flags(eventFlags[i]),
                          inode);

#else
      events.emplace_back(((char **) eventPaths)[i],
                          curr_time,
                          decode_flags(eventFlags[i]));
#endif
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
    auto context = std::make_unique<FSEventStreamContext>();
    context->version = 0;
    context->info = this;
    context->retain = nullptr;
    context->release = nullptr;
    context->copyDescription = nullptr;

    FSEventStreamCreateFlags streamFlags = kFSEventStreamCreateFlagNone;
    if (this->no_defer()) streamFlags |= kFSEventStreamCreateFlagNoDefer;
#ifdef HAVE_MACOS_GE_10_7
    streamFlags |= kFSEventStreamCreateFlagFileEvents;
#endif

#ifdef HAVE_MACOS_GE_10_13
    streamFlags |= kFSEventStreamCreateFlagUseExtendedData;
    streamFlags |= kFSEventStreamCreateFlagUseCFTypes;
#endif

    FSW_ELOG(_("Creating FSEvent stream...\n"));
    pImpl->stream = FSEventStreamCreate(nullptr,
                                 &fsevents_monitor::fsevents_callback,
                                 context.get(),
                                 pathsToWatch,
                                 kFSEventStreamEventIdSinceNow,
                                 latency,
                                 streamFlags);
  }
}
