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
#include "fsevent_monitor.h"

#ifdef HAVE_CORESERVICES_CORESERVICES_H

#include "fswatch_exception.h"
#include "fswatch_log.h"
#include <iostream>
#include "event.h"

using namespace std;

typedef struct FSEventFlagType
{
  FSEventStreamEventFlags flag;
  event_flag type;
} FSEventFlagType;

static const vector<FSEventFlagType> event_flag_type = {
  { kFSEventStreamEventFlagNone, event_flag::PlatformSpecific},
  { kFSEventStreamEventFlagMustScanSubDirs, event_flag::PlatformSpecific},
  { kFSEventStreamEventFlagUserDropped, event_flag::PlatformSpecific},
  { kFSEventStreamEventFlagKernelDropped, event_flag::PlatformSpecific},
  { kFSEventStreamEventFlagEventIdsWrapped, event_flag::PlatformSpecific},
  { kFSEventStreamEventFlagHistoryDone, event_flag::PlatformSpecific},
  { kFSEventStreamEventFlagRootChanged, event_flag::PlatformSpecific},
  { kFSEventStreamEventFlagMount, event_flag::PlatformSpecific},
  { kFSEventStreamEventFlagUnmount, event_flag::PlatformSpecific},
  { kFSEventStreamEventFlagItemCreated, event_flag::Created},
  { kFSEventStreamEventFlagItemRemoved, event_flag::Removed},
  { kFSEventStreamEventFlagItemInodeMetaMod, event_flag::PlatformSpecific},
  { kFSEventStreamEventFlagItemRenamed, event_flag::Renamed},
  { kFSEventStreamEventFlagItemModified, event_flag::Updated},
  { kFSEventStreamEventFlagItemFinderInfoMod, event_flag::PlatformSpecific},
  { kFSEventStreamEventFlagItemChangeOwner, event_flag::OwnerModified},
  { kFSEventStreamEventFlagItemXattrMod, event_flag::AttributeModified},
  { kFSEventStreamEventFlagItemIsFile, event_flag::IsFile},
  { kFSEventStreamEventFlagItemIsDir, event_flag::IsDir},
  { kFSEventStreamEventFlagItemIsSymlink, event_flag::IsSymLink}
};

fsevent_monitor::fsevent_monitor(vector<string> paths_to_monitor,
                                 EVENT_CALLBACK callback) :
  monitor(paths_to_monitor, callback)
{
}

fsevent_monitor::~fsevent_monitor()
{
  if (stream)
  {
    fsw_log("Stopping event stream...\n");
    FSEventStreamStop(stream);

    fsw_log("Invalidating event stream...\n");
    FSEventStreamInvalidate(stream);

    fsw_log("Releasing event stream...\n");
    FSEventStreamRelease(stream);
  }

  stream = nullptr;
}

void fsevent_monitor::set_numeric_event(bool numeric)
{
  numeric_event = numeric;
}

void fsevent_monitor::run()
{
  if (stream) return;

  // parsing paths
  vector<CFStringRef> dirs;

  for (string path : paths)
  {
    if (accept_path(path))
    {
      dirs.push_back(CFStringCreateWithCString(NULL,
                                               path.c_str(),
                                               kCFStringEncodingUTF8));
    }
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

  fsw_log("Creating FSEvent stream...\n");
  stream = FSEventStreamCreate(NULL,
                               &fsevent_monitor::fsevent_callback,
                               context,
                               pathsToWatch,
                               kFSEventStreamEventIdSinceNow,
                               latency,
                               kFSEventStreamCreateFlagFileEvents);

  if (!stream)
  {
    throw fsw_exception("Event stream could not be created.");
  }

  fsw_log("Scheduling stream with run loop...\n");
  FSEventStreamScheduleWithRunLoop(stream,
                                   CFRunLoopGetCurrent(),
                                   kCFRunLoopDefaultMode);

  fsw_log("Starting event stream...\n");
  FSEventStreamStart(stream);

  fsw_log("Starting run loop...\n");
  CFRunLoopRun();
}

static vector<event_flag> decode_flags(FSEventStreamEventFlags flag)
{
  vector<event_flag> evt_flags;

  for (FSEventFlagType type : event_flag_type)
  {
    if (flag & type.flag)
    {
      evt_flags.push_back(type.type);
    }
  }

  return evt_flags;
}

void fsevent_monitor::fsevent_callback(ConstFSEventStreamRef streamRef,
                                       void *clientCallBackInfo,
                                       size_t numEvents,
                                       void *eventPaths,
                                       const FSEventStreamEventFlags eventFlags[],
                                       const FSEventStreamEventId eventIds[])
{
  fsevent_monitor *fse_monitor =
    reinterpret_cast<fsevent_monitor *> (clientCallBackInfo);

  if (!fse_monitor)
  {
    throw fsw_exception("The callback info cannot be cast to fsevent_monitor.");
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
    fse_monitor->callback(events);
  }
}

#endif  /* HAVE_CORESERVICES_CORESERVICES_H */
