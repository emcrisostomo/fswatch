/*
 * Copyright (c) 2026 Enrico M. Crisostomo
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

#include <iostream>
#include <string>
#include <vector>

#include <libfswatch/c++/event.hpp>
#include <libfswatch/c++/libfswatch_exception.hpp>
#include <libfswatch/c++/monitor.hpp>
#include <libfswatch/c/error.h>
#include <libfswatch/c/libfswatch.h>

using namespace fsw;

namespace
{
  void test_callback(const std::vector<event>&, void *)
  {
  }

  class test_monitor : public monitor
  {
  public:
    test_monitor() : monitor({"."}, test_callback)
    {
    }

    bool accepts(const std::string& path) const
    {
      return accept_path(path);
    }

  private:
    void run() override
    {
    }
  };

  bool expect(bool condition, const char *message)
  {
    if (condition) return true;

    std::cerr << message << "\n";
    return false;
  }

  monitor_filter include(const std::string& pattern)
  {
    return {pattern, fsw_filter_type::filter_include, true, false};
  }

  monitor_filter exclude(const std::string& pattern)
  {
    return {pattern, fsw_filter_type::filter_exclude, true, false};
  }
}

int main()
{
  bool ok = true;

  test_monitor legacy;
  legacy.add_filter(exclude(".*"));
  legacy.add_filter(include("\\.c$"));
  legacy.add_filter(exclude("/skip\\.c$"));

  ok = expect(legacy.accepts("/tmp/keep.c"),
              "legacy mode rejected an included path") && ok;
  ok = expect(legacy.accepts("/tmp/skip.c"),
              "legacy mode did not let include override exclude") && ok;
  ok = expect(!legacy.accepts("/tmp/other.txt"),
              "legacy mode accepted an excluded unmatched path") && ok;

  test_monitor conjunctive;
  conjunctive.set_filter_mode(fsw_filter_mode::filter_mode_conjunctive);
  conjunctive.add_filter(include("\\.c$"));
  conjunctive.add_filter(exclude("/skip\\.c$"));

  ok = expect(conjunctive.accepts("/tmp/keep.c"),
              "conjunctive mode rejected an included path") && ok;
  ok = expect(!conjunctive.accepts("/tmp/skip.c"),
              "conjunctive mode did not subtract an excluded included path") &&
       ok;
  ok = expect(!conjunctive.accepts("/tmp/other.txt"),
              "conjunctive mode accepted an excluded unmatched path") && ok;

  test_monitor include_only;
  include_only.set_filter_mode(fsw_filter_mode::filter_mode_conjunctive);
  include_only.add_filter(include("\\.c$"));

  ok = expect(include_only.accepts("/tmp/keep.c"),
              "conjunctive include-only mode rejected a matching path") && ok;
  ok = expect(!include_only.accepts("/tmp/other.txt"),
              "conjunctive include-only mode accepted a nonmatching path") &&
       ok;

  test_monitor exclude_only;
  exclude_only.set_filter_mode(fsw_filter_mode::filter_mode_conjunctive);
  exclude_only.add_filter(exclude("/skip\\.c$"));

  ok = expect(exclude_only.accepts("/tmp/keep.c"),
              "conjunctive exclude-only mode rejected an unmatched path") && ok;
  ok = expect(!exclude_only.accepts("/tmp/skip.c"),
              "conjunctive exclude-only mode accepted a matching path") && ok;

  try
  {
    test_monitor invalid;
    invalid.set_filter_mode(static_cast<fsw_filter_mode>(-1));
    ok = expect(false, "invalid filter mode was accepted") && ok;
  }
  catch (const libfsw_exception& ex)
  {
    ok = expect(static_cast<int>(ex) == FSW_ERR_UNKNOWN_VALUE,
                "invalid filter mode reported the wrong error") && ok;
  }

  FSW_HANDLE handle = fsw_init_session(fsw_monitor_type::poll_monitor_type);
  ok = expect(handle != nullptr, "C API session could not be created") && ok;
  if (handle != nullptr)
  {
    ok = expect(fsw_set_filter_mode(
                  handle,
                  fsw_filter_mode::filter_mode_conjunctive) == FSW_OK,
                "C API rejected conjunctive filter mode") && ok;
    ok = expect(fsw_set_filter_mode(
                  handle,
                  static_cast<fsw_filter_mode>(-1)) == FSW_ERR_UNKNOWN_VALUE,
                "C API accepted invalid filter mode") && ok;
    ok = expect(fsw_destroy_session(handle) == FSW_OK,
                "C API session could not be destroyed") && ok;
  }

  return ok ? 0 : 1;
}
