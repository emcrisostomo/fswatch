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

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <future>
#include <iostream>
#include <libfswatch/c/libfswatch.h>
#include <string>
#include <thread>

namespace
{
  void callback(fsw_cevent const *const, const unsigned int, void *)
  {
  }

  bool expect_ok(FSW_STATUS status, const char *message)
  {
    if (status == FSW_OK) return true;

    std::cerr << message << ": " << fsw_last_error() << "\n";
    return false;
  }
}

int main()
{
  namespace fs = std::filesystem;
  using namespace std::chrono_literals;

  if (!expect_ok(fsw_init_library(), "fsw_init_library failed")) return 1;

  const fs::path test_dir =
    fs::temp_directory_path() /
    ("fswatch-inotify-stop-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  fs::create_directory(test_dir);

  FSW_HANDLE handle = fsw_init_session(inotify_monitor_type);
  if (!handle)
  {
    std::cerr << "fsw_init_session failed\n";
    fs::remove_all(test_dir);
    return 1;
  }

  bool ok =
    expect_ok(fsw_add_path(handle, test_dir.string().c_str()), "fsw_add_path failed") &&
    expect_ok(fsw_set_callback(handle, callback, nullptr), "fsw_set_callback failed") &&
    expect_ok(fsw_set_latency(handle, 30.0), "fsw_set_latency failed");

  if (!ok)
  {
    fsw_destroy_session(handle);
    fs::remove_all(test_dir);
    return 1;
  }

  auto monitor_status = std::async(std::launch::async, [handle] {
    return fsw_start_monitor(handle);
  });

  std::this_thread::sleep_for(500ms);

  const auto stop_started = std::chrono::steady_clock::now();
  if (!expect_ok(fsw_stop_monitor(handle), "fsw_stop_monitor failed"))
  {
    std::quick_exit(1);
  }

  if (monitor_status.wait_for(2s) != std::future_status::ready)
  {
    std::cerr << "inotify monitor did not stop promptly with long latency\n";
    std::quick_exit(2);
  }

  if (!expect_ok(monitor_status.get(), "fsw_start_monitor failed"))
  {
    fsw_destroy_session(handle);
    fs::remove_all(test_dir);
    return 1;
  }

  const auto stop_elapsed = std::chrono::steady_clock::now() - stop_started;
  if (stop_elapsed > 2s)
  {
    std::cerr << "inotify monitor stop took too long\n";
    fsw_destroy_session(handle);
    fs::remove_all(test_dir);
    return 1;
  }

  if (!expect_ok(fsw_destroy_session(handle), "fsw_destroy_session failed"))
  {
    fs::remove_all(test_dir);
    return 1;
  }

  fs::remove_all(test_dir);
  return 0;
}
