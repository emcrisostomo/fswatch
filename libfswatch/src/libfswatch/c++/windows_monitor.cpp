/*
 * Copyright (c) 2014-2015 Enrico M. Crisostomo
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

#ifdef HAVE_WINDOWS

#  include "gettext_defs.h"
#  include "windows_monitor.hpp"
#  include "libfswatch_map.hpp"
#  include "libfswatch_set.hpp"
#  include "libfswatch_exception.hpp"
#  include "../c/libfswatch_log.h"
#  include <algorithm>
#  include <set>
#  include <iostream>
#  include <memory>
#  include <sys/types.h>
#  include <cstdlib>
#  include <cstring>
#  include <ctime>
#  include <cstdio>
#  include <unistd.h>
#  include <fcntl.h>
#  include <windows.h>
#  include "./windows/win_handle.hpp"
#  include "./windows/win_error_message.hpp"
#  include "./windows/win_strings.hpp"
#  include "./windows/win_paths.hpp"
#  include "./windows/win_directory_change_event.hpp"

using namespace std;

namespace fsw
{
  struct windows_monitor_load
  {
    fsw_hash_set<wstring> win_paths;
    fsw_hash_map<wstring, directory_change_event> dce_by_path;
    fsw_hash_map<wstring, win_handle> event_by_path;
    long buffer_size = 128;
  };

  windows_monitor::windows_monitor(vector<string> paths_to_monitor,
                                   FSW_EVENT_CALLBACK * callback,
                                   void * context) :
    monitor(paths_to_monitor, callback, context), load(new windows_monitor_load())
  {
    SetConsoleOutputCP(CP_UTF8);
  }

  windows_monitor::~windows_monitor()
  {
    delete load;
  }

  void windows_monitor::initialize_windows_path_list()
  {
    for (const auto & path : paths)
    {
      load->win_paths.insert(win_paths::posix_to_win_w(path));
    }
  }

  void windows_monitor::initialize_events()
  {
    for (const wstring & path : load->win_paths)
    {
      FSW_ELOGF(_("Creating event for %s.\n"), win_strings::wstring_to_string(path).c_str());

      HANDLE h = CreateEvent(nullptr,
                             TRUE,
                             FALSE,
                             nullptr);

      if (h == NULL) throw libfsw_exception(_("CreateEvent failed."));

      FSW_ELOGF(_("Event %d created for %s.\n"), h, win_strings::wstring_to_string(path).c_str());

      load->event_by_path.emplace(path, h);
    }
  }

  bool windows_monitor::init_search_for_path(const wstring path)
  {
    FSW_ELOGF(_("Initializing search structures for %s.\n"), win_strings::wstring_to_string(path).c_str());

    HANDLE h = CreateFileW(path.c_str(),
                           GENERIC_READ,
                           FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                           nullptr, OPEN_EXISTING,
                           FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                           nullptr);

    if (!win_handle::is_valid(h))
    {
      fprintf(stderr, _("Invalid handle when opening %s.\n"), win_strings::wstring_to_string(path).c_str());
      return false;
    }

    FSW_ELOGF(_("Open file handle: %d.\n"), h);

    directory_change_event dce(load->buffer_size);
    dce.path = path;
    dce.handle = h;
    dce.overlapped.get()->hEvent = load->event_by_path[path];

    if (!dce.read_changes_async())
    {
      FSW_ELOGF("ReadDirectoryChangesW: %s\n", win_strings::wstring_to_string(win_error_message::current()).c_str());
      return false;
    }

    load->dce_by_path[path] = move(dce);

    return true;
  }

  void windows_monitor::stop_search_for_path(const wstring path)
  {
    load->dce_by_path.erase(path);
  }

  bool windows_monitor::is_path_watched(wstring path)
  {
    return (load->dce_by_path.find(path) != load->dce_by_path.end());
  }

  void windows_monitor::process_path(const wstring & path)
  {
    FSW_ELOGF(_("Processing %s.\n"), win_strings::wstring_to_string(path).c_str());

    // If the path is not currently watched, then initialize the search
    // structures.  If the initalization fails, skip the path altogether
    // until the next iteration.
    if (!is_path_watched(path))
    {
      if (!init_search_for_path(path)) return;
    }

    auto it = load->dce_by_path.find(path);
    if (it == load->dce_by_path.end()) throw libfsw_exception(_("Initialization failed."));

    directory_change_event & dce = it->second;

    if (!dce.try_read())
    {
      if (dce.is_io_incomplete())
      {
        FSW_ELOG(_("I/O incomplete.\n"));
        return;
      }

      if (dce.is_buffer_overflowed())
      {
        notify_overflow(win_paths::win_w_to_posix(path));
      }

      stop_search_for_path(path);

      return;
    }

    FSW_ELOGF(_("GetOverlappedResult returned %d bytes\n"), dce.bytes_returned);

    if (dce.bytes_returned == 0)
    {
      notify_overflow(win_paths::win_w_to_posix(path));
    }
    else
    {
      vector<event> events = dce.get_events();

      if (events.size()) notify_events(events);
    }

    if (!dce.read_changes_async())
    {
      FSW_ELOGF(_("ReadDirectoryChangesW: %s\n"), win_strings::wstring_to_string(win_error_message::current()).c_str());
      stop_search_for_path(path);
    }
  }

  void windows_monitor::configure_monitor()
  {
    string buffer_size_value = get_property("windows.ReadDirectoryChangesW.buffer.size");

    if (buffer_size_value.empty()) return;

    long parsed_value = strtol(buffer_size_value.c_str(), nullptr, 0);

    if (parsed_value <= 0)
    {
      string msg = string(_("Invalid value: ")) + buffer_size_value;
      throw libfsw_exception(msg.c_str());
    }

    load->buffer_size = parsed_value;
  }

  void windows_monitor::run()
  {
    // Since the file handles are open with FILE_SHARE_DELETE, it may happen
    // that file is deleted when a handle to it is being used.  A call to
    // either ReadDirectoryChangesW or GetOverlappedResult will return with
    // an error if the file system object being observed is deleted.
    // Unfortunately, the error reported by Windows is `Access denied',
    // preventing fswatch to report better messages to the user.

    configure_monitor();
    initialize_windows_path_list();
    initialize_events();

    for (;;)
    {
#ifdef HAVE_CXX_MUTEX
      unique_lock<mutex> run_guard(run_mutex);
      if (should_stop) break;
      run_guard.unlock();
#endif

      sleep(latency);

      for (const auto & path : load->win_paths)
      {
        process_path(path);
      }
    }
  }
}

#endif  /* HAVE_WINDOWS */
