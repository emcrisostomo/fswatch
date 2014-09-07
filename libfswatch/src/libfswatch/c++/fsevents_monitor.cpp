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

#include "fsevents_monitor.h"
#include "gettext_defs.h"
#include "libfswatch_exception.h"
#include "c/libfswatch_log.h"
#include <iostream>
#include "event.h"

using namespace std;

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
                                     FSW_EVENT_CALLBACK * callback,
                                     void * context) :
    monitor(paths_to_monitor, callback, context)
  {
  }

  fsevents_monitor::~fsevents_monitor()
  {
    if (stream)
    {
      libfsw_log(_("Stopping event stream...\n"));
      FSEventStreamStop(stream);

      libfsw_log(_("Invalidating event stream...\n"));
      FSEventStreamInvalidate(stream);

      libfsw_log(_("Releasing event stream...\n"));
      FSEventStreamRelease(stream);
    }

    stream = nullptr;
  }

  void fsevents_monitor::set_numeric_event(bool numeric)
  {
    numeric_event = numeric;
  }

  void fsevents_monitor::run()
  {
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

    libfsw_log(_("Creating FSEvent stream...\n"));
    stream = FSEventStreamCreate(NULL,
                                 &fsevents_monitor::fsevents_callback,
                                 context,
                                 pathsToWatch,
                                 kFSEventStreamEventIdSinceNow,
                                 latency,
                                 kFSEventStreamCreateFlagFileEvents);

    if (!stream)
    {
      throw libfsw_exception(_("Event stream could not be created."));
    }

    libfsw_log(_("Scheduling stream with run loop...\n"));
    FSEventStreamScheduleWithRunLoop(stream,
                                     CFRunLoopGetCurrent(),
                                     kCFRunLoopDefaultMode);

    libfsw_log(_("Starting event stream...\n"));
    FSEventStreamStart(stream);

    libfsw_log(_("Starting run loop...\n"));
    CFRunLoopRun();
  }

  static vector<fsw_event_flag> decode_flags(FSEventStreamEventFlags flag)
  {
    vector<fsw_event_flag> evt_flags;

    for (FSEventFlagType type : event_flag_type)
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
    fsevents_monitor *fse_monitor =
      static_cast<fsevents_monitor *> (clientCallBackInfo);

    if (!fse_monitor)
    {
      throw libfsw_exception(_("The callback info cannot be cast to fsevents_monitor."));
    }

    vector<event> events;

    time_t curr_time;
    time(&curr_time);

    for (size_t i = 0; i < numEvents; ++i)
    {
      const char * path = ((char **) eventPaths)[i];

      if (!fse_monitor->accept_path(path)) continue;

      events.push_back({path, curr_time, decode_flags(eventFlags[i])});
    }

    if (events.size() > 0)
    {
      fse_monitor->callback(events, fse_monitor->context);
    }
  }
}
