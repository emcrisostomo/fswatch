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
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <libfswatch/c/libfswatch.h>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace
{
  struct callback_context
  {
    std::mutex mutex;
    std::condition_variable condition;
    std::vector<std::string> paths;
  };

  void callback(fsw_cevent const *const events,
                const unsigned int event_num,
                void *data)
  {
    auto *context = static_cast<callback_context *>(data);

    {
      std::lock_guard<std::mutex> guard(context->mutex);
      for (unsigned int i = 0; i < event_num; ++i)
      {
        context->paths.emplace_back(events[i].path);
      }
    }

    context->condition.notify_all();
  }

  bool expect_ok(FSW_STATUS status, const char *message)
  {
    if (status == FSW_OK) return true;

    std::cerr << message << ": " << fsw_last_error() << "\n";
    return false;
  }

  bool has_path_containing(const callback_context& context,
                           const std::string& needle)
  {
    for (const auto& path : context.paths)
    {
      if (path.find(needle) != std::string::npos) return true;
    }

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
    ("fswatch-prune-c-api-" +
     std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  const fs::path pruned_dir = test_dir / "pruned";
  const fs::path visible_dir = test_dir / "visible";

  fs::create_directories(pruned_dir);
  fs::create_directories(visible_dir);

  FSW_HANDLE handle = fsw_init_session(poll_monitor_type);
  if (!handle)
  {
    std::cerr << "fsw_init_session failed\n";
    fs::remove_all(test_dir);
    return 1;
  }

  callback_context context;
  char prune_pattern[] = "/pruned$";
  fsw_cmonitor_filter prune_filter = {
    prune_pattern,
    fsw_filter_type::filter_exclude,
    true,
    false,
  };

  bool ok =
    expect_ok(fsw_add_path(handle, test_dir.string().c_str()), "fsw_add_path failed") &&
    expect_ok(fsw_set_recursive(handle, true), "fsw_set_recursive failed") &&
    expect_ok(fsw_set_callback(handle, callback, &context), "fsw_set_callback failed") &&
    expect_ok(fsw_set_latency(handle, 1.0), "fsw_set_latency failed") &&
    expect_ok(fsw_add_prune_filter(handle, prune_filter), "fsw_add_prune_filter failed");

  if (!ok)
  {
    fsw_destroy_session(handle);
    fs::remove_all(test_dir);
    return 1;
  }

  auto monitor_status = std::async(std::launch::async, [handle] {
    return fsw_start_monitor(handle);
  });

  std::this_thread::sleep_for(1500ms);

  {
    std::ofstream visible_file(visible_dir / "visible.txt");
    visible_file << "visible\n";

    std::ofstream hidden_file(pruned_dir / "hidden.txt");
    hidden_file << "hidden\n";
  }

  {
    std::unique_lock<std::mutex> guard(context.mutex);
    context.condition.wait_for(guard, 6s, [&context] {
      return has_path_containing(context, "/visible/visible.txt");
    });
  }

  if (!expect_ok(fsw_stop_monitor(handle), "fsw_stop_monitor failed"))
  {
    std::quick_exit(1);
  }

  if (monitor_status.wait_for(3s) != std::future_status::ready)
  {
    std::cerr << "poll monitor did not stop promptly\n";
    std::quick_exit(2);
  }

  ok = expect_ok(monitor_status.get(), "fsw_start_monitor failed") && ok;

  {
    std::lock_guard<std::mutex> guard(context.mutex);
    ok = (has_path_containing(context, "/visible/visible.txt") ||
          (std::cerr << "missing event outside pruned directory\n", false)) && ok;
    ok = (!has_path_containing(context, "/pruned/hidden.txt") ||
          (std::cerr << "event below pruned directory was reported\n", false)) && ok;
  }

  ok = expect_ok(fsw_destroy_session(handle), "fsw_destroy_session failed") && ok;

  fs::remove_all(test_dir);
  return ok ? 0 : 1;
}
