/*
 * Copyright (c) 2014-2026 Enrico M. Crisostomo
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

#ifdef HAVE_MACOS_GE_10_13

#include <CoreServices/CoreServices.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <libfswatch/c++/monitor.hpp>

// Expose the callback for a focused regression test without changing the API.
#define private public
#include <libfswatch/c++/fsevents_monitor.hpp>
#undef private

namespace
{
  std::vector<fsw::event> received_events;

  void capture_events(const std::vector<fsw::event>& events, void *)
  {
    received_events.insert(received_events.end(), events.begin(), events.end());
  }

  bool has_flag(const fsw::event& evt, fsw_event_flag flag)
  {
    const auto flags = evt.get_flags();
    return std::find(flags.begin(), flags.end(), flag) != flags.end();
  }
}

int main()
{
  const std::string expected_path = "/tmp/fswatch-fsevents-missing-inode";

  auto path = CFStringCreateWithCString(nullptr,
                                        expected_path.c_str(),
                                        kCFStringEncodingUTF8);
  if (!path)
  {
    std::cerr << "Failed to create CFString path." << std::endl;
    return EXIT_FAILURE;
  }

  const void *keys[] = {kFSEventStreamEventExtendedDataPathKey};
  const void *values[] = {path};
  auto event_info = CFDictionaryCreate(nullptr,
                                       keys,
                                       values,
                                       1,
                                       &kCFTypeDictionaryKeyCallBacks,
                                       &kCFTypeDictionaryValueCallBacks);
  CFRelease(path);

  if (!event_info)
  {
    std::cerr << "Failed to create FSEvents event dictionary." << std::endl;
    return EXIT_FAILURE;
  }

  const void *event_values[] = {event_info};
  auto event_paths = CFArrayCreate(nullptr,
                                   event_values,
                                   1,
                                   &kCFTypeArrayCallBacks);
  CFRelease(event_info);

  if (!event_paths)
  {
    std::cerr << "Failed to create FSEvents event array." << std::endl;
    return EXIT_FAILURE;
  }

  fsw::fsevents_monitor monitor({expected_path}, capture_events);
  const FSEventStreamEventFlags event_flags[] = {kFSEventStreamEventFlagItemRemoved};
  const FSEventStreamEventId event_ids[] = {1};

  fsw::fsevents_monitor::fsevents_callback(nullptr,
                                           &monitor,
                                           1,
                                           const_cast<void *>(static_cast<const void *>(event_paths)),
                                           event_flags,
                                           event_ids);
  CFRelease(event_paths);

  if (received_events.size() != 1)
  {
    std::cerr << "Expected one event, got " << received_events.size() << "." << std::endl;
    return EXIT_FAILURE;
  }

  const auto& event = received_events.front();
  if (event.get_path() != expected_path)
  {
    std::cerr << "Expected path " << expected_path << ", got " << event.get_path() << "." << std::endl;
    return EXIT_FAILURE;
  }

  if (event.get_correlation_id() != 0)
  {
    std::cerr << "Expected missing file ID to produce correlation id 0, got "
              << event.get_correlation_id() << "." << std::endl;
    return EXIT_FAILURE;
  }

  if (!has_flag(event, Removed))
  {
    std::cerr << "Expected Removed event flag." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

#else

int main()
{
  return 77;
}

#endif
