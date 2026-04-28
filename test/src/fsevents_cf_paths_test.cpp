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

#include <CoreServices/CoreServices.h>
#include <cstdlib>
#include <iostream>
#include <limits.h>
#include <string>
#include <vector>

#include <libfswatch/c++/monitor.hpp>

#define private public
#include <libfswatch/c++/fsevents_monitor.hpp>
#undef private

namespace
{
  bool cf_string_equals(CFStringRef value, const std::string& expected)
  {
    char buffer[PATH_MAX];
    return CFStringGetCString(value, buffer, sizeof(buffer), kCFStringEncodingUTF8)
           && expected == buffer;
  }
}

int main()
{
  const std::vector<std::string> expected_paths = {"/tmp", "/var"};
  CFArrayRef paths = fsw::fsevents_monitor::copy_cf_paths(expected_paths);

  if (!paths)
  {
    std::cerr << "Expected path array to be created." << std::endl;
    return EXIT_FAILURE;
  }

  if (CFArrayGetCount(paths) != static_cast<CFIndex>(expected_paths.size()))
  {
    std::cerr << "Unexpected path count." << std::endl;
    CFRelease(paths);
    return EXIT_FAILURE;
  }

  for (CFIndex i = 0; i < CFArrayGetCount(paths); ++i)
  {
    auto path = static_cast<CFStringRef>(CFArrayGetValueAtIndex(paths, i));
    if (!cf_string_equals(path, expected_paths[i]))
    {
      std::cerr << "Unexpected path at index " << i << "." << std::endl;
      CFRelease(paths);
      return EXIT_FAILURE;
    }
  }

  CFRelease(paths);

  if (fsw::fsevents_monitor::copy_cf_paths({}) != nullptr)
  {
    std::cerr << "Empty path input must not create an array." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
