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
#  include <sys/cygwin.h>
#  include <windows.h>
#  include "./windows/win_handle.hpp"
#  include "./windows/win_error_message.hpp"

using namespace std;

namespace fsw
{
  REGISTER_MONITOR_IMPL(windows_monitor, windows_monitor_type);

  static string wstring_to_string(wchar_t * s)
  {
    int buf_size = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
    char buf[buf_size];
    WideCharToMultiByte(CP_UTF8, 0, s, -1, buf, buf_size, NULL, NULL);

    return string(buf);
  }

  static string wstring_to_string(const wstring & s)
  {
    return wstring_to_string((wchar_t *)s.c_str());
  }

  static wstring posix_to_win_w(string path)
  {
    void * raw_path = ::cygwin_create_path(CCP_POSIX_TO_WIN_W, path.c_str());
    if (raw_path == nullptr) throw libfsw_exception(_("cygwin_create_path could not allocate memory to convert the path."));

    wstring win_path(static_cast<wchar_t *>(raw_path));

    ::free(raw_path);

    return win_path;
  }

  static string win_w_to_posix(wstring path)
  {
    void * raw_path = ::cygwin_create_path(CCP_WIN_W_TO_POSIX, path.c_str());
    if (raw_path == nullptr) throw libfsw_exception(_("cygwin_create_path could not allocate memory to convert the path."));

    string posix_path(static_cast<char *>(raw_path));

    ::free(raw_path);

    return posix_path;
  }

  struct win_flag_type
  {
    DWORD action;
    vector<fsw_event_flag> types;
  };

  static vector<win_flag_type> create_flag_type_vector()
  {
    vector<win_flag_type> flags;
    flags.push_back({FILE_ACTION_ADDED,            {fsw_event_flag::Created}});
    flags.push_back({FILE_ACTION_REMOVED,          {fsw_event_flag::Removed}});
    flags.push_back({FILE_ACTION_MODIFIED,         {fsw_event_flag::Updated}});
    flags.push_back({FILE_ACTION_RENAMED_OLD_NAME, {fsw_event_flag::MovedFrom, fsw_event_flag::Renamed}});
    flags.push_back({FILE_ACTION_RENAMED_NEW_NAME, {fsw_event_flag::MovedTo, fsw_event_flag::Renamed}});

    return flags;
  }

  static const vector<win_flag_type> event_flag_type = create_flag_type_vector();

  static vector<fsw_event_flag> decode_flags(DWORD flag)
  {
    set<fsw_event_flag> evt_flags_set;

    for (const win_flag_type & event_type : event_flag_type)
    {
      if (flag == event_type.action)
      {
        for (const auto & type : event_type.types) evt_flags_set.insert(type);
      }
    }

    return vector<fsw_event_flag>(evt_flags_set.begin(), evt_flags_set.end());
  }

  typedef struct directory_change_event
  {
    wstring path;
    win_handle handle;
    size_t buffer_size;
    DWORD bytes_returned;
    unique_ptr<void, decltype(::free)*> buffer = {nullptr, ::free};
    unique_ptr<OVERLAPPED, decltype(::free)*> overlapped = {static_cast<OVERLAPPED *>(::malloc(sizeof(OVERLAPPED))), ::free};
    win_error_message read_error;

    directory_change_event(size_t buffer_length = 16) : handle{INVALID_HANDLE_VALUE},
                                                        buffer_size{sizeof(FILE_NOTIFY_INFORMATION) * buffer_length},
                                                        bytes_returned{}
    {
      buffer.reset(::malloc(buffer_size));
      if (buffer.get() == nullptr) throw libfsw_exception(_("::malloc failed."));
      if (overlapped.get() == nullptr) throw libfsw_exception(_("::malloc failed."));
    }

    bool is_io_incomplete()
    {
      return (read_error.get_error_code() == ERROR_IO_INCOMPLETE);
    }

    bool is_buffer_overflowed()
    {
      return (read_error.get_error_code() == ERROR_NOTIFY_ENUM_DIR);
    }

    bool read_changes_async()
    {
      continue_read();

      FSW_LOGF(_("%p.\n"), this);

      return ReadDirectoryChangesW((HANDLE)handle,
                                   buffer.get(),
                                   buffer_size,
                                   TRUE,
                                   FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                   FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION,
                                   &bytes_returned,
                                   overlapped.get(),
                                   nullptr);
    }

    bool try_read()
    {
      bool ret = GetOverlappedResult(handle, overlapped.get(), &bytes_returned, FALSE);

      read_error = win_error_message::current();

      FSW_LOGF(_("GetOverlappedResult: %s\n"), wstring_to_string((wstring)read_error).c_str());

      return ret;
    }

    void continue_read()
    {
      if (!ResetEvent(overlapped.get()->hEvent)) throw libfsw_exception(_("::ResetEvent failed."));

      FSW_LOGF(_("Event %d reset.\n"), overlapped.get()->hEvent);
    }

    vector<event> get_events()
    {
      // TO DO: We are relying on callers to know events are ready.
      vector<event> events;

      time_t curr_time;
      time(&curr_time);

      char * curr_entry = static_cast<char *>(buffer.get());

      while (curr_entry != nullptr)
      {
        FILE_NOTIFY_INFORMATION * currEntry = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(curr_entry);

        if (currEntry->FileNameLength > 0)
        {
          // The FileName member of the FILE_NOTIFY_INFORMATION structure
          // has the following characteristics:
          //
          //   * It's not NUL terminated.
          //
          //   * Its length is specified in bytes.
          wstring file_name = path
            + L"\\"
            + wstring(currEntry->FileName, currEntry->FileNameLength/sizeof(wchar_t));

          events.push_back({win_w_to_posix(file_name), curr_time, decode_flags(currEntry->Action)});
        }
        else
        {
          cerr << _("File name unexpectedly empty.") << endl;
        }

        curr_entry = (currEntry->NextEntryOffset == 0) ? nullptr : curr_entry + currEntry->NextEntryOffset;
      }

      return events;
    }
  } directory_change_event;

  struct windows_monitor_load
  {
    fsw_hash_set<wstring> win_paths;
    fsw_hash_map<wstring, directory_change_event> dce_by_path;
    fsw_hash_map<wstring, win_handle> event_by_path;
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
      load->win_paths.insert(posix_to_win_w(path));
    }
  }

  void windows_monitor::initialize_events()
  {
    for (const wstring & path : load->win_paths)
    {
      FSW_LOGF(_("Creating event for %s.\n"), wstring_to_string(path).c_str());

      HANDLE h = ::CreateEvent(nullptr,
                               TRUE,
                               FALSE,
                               nullptr);

      if (h == NULL) throw libfsw_exception(_("CreateEvent failed."));

      FSW_LOGF(_("Event %d created for %s.\n"), h, wstring_to_string(path).c_str());

      load->event_by_path.emplace(path, h);
    }
  }

  bool windows_monitor::init_search_for_path(const wstring path)
  {
    FSW_LOGF(_("Initializing search structures for %s.\n"), wstring_to_string(path).c_str());

    HANDLE h = ::CreateFileW(path.c_str(),
                             GENERIC_READ,
                             FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                             nullptr, OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                             nullptr);

    if (!win_handle::is_valid(h))
    {
      fprintf(stderr, _("Invalid handle when opening %s.\n"), wstring_to_string(path).c_str());
      return false;
    }

    FSW_LOGF(_("Open file handle: %d.\n"), h);

    directory_change_event dce(128);
    dce.path = path;
    dce.handle = h;
    dce.overlapped.get()->hEvent = load->event_by_path[path];

    if (!dce.read_changes_async())
    {
      FSW_LOGF("ReadDirectoryChangesW: %s\n", wstring_to_string(win_error_message::current()).c_str());
      return false;
    }

    load->dce_by_path[path] = std::move(dce);

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
    FSW_LOGF(_("Processing %s.\n"), wstring_to_string(path).c_str());

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
        FSW_LOG(_("I/O incomplete.\n"));
        return;
      }

      if (dce.is_buffer_overflowed())
      {
        cerr << "Overflow." << endl;
      }

      stop_search_for_path(path);

      return;
    }

    FSW_LOGF(_("GetOverlappedResult returned %d bytes\n"), dce.bytes_returned);

    if(dce.bytes_returned == 0)
    {
      throw libfsw_exception(_("Event queue overflow."));
    }

    vector<event> events = dce.get_events();

    if (events.size()) notify_events(events);

    if (!dce.read_changes_async())
    {
      FSW_LOGF(_("ReadDirectoryChangesW: %s\n"), wstring_to_string(win_error_message::current()).c_str());
      stop_search_for_path(path);
    }
  }

  void windows_monitor::run()
  {
    // Since the file handles are open with FILE_SHARE_DELETE, it may happen
    // that file is deleted when a handle to it is being used.  A call to
    // either ReadDirectoryChangesW or GetOverlappedResult will return with
    // an error if the file system object being observed is deleted.
    // Unfortunately, the error reported by Windows is `Access denied',
    // preventing fswatch to report better messages to the user.

    initialize_windows_path_list();
    initialize_events();

    while (true)
    {
      ::sleep(latency);

      for (const auto & path : load->win_paths)
      {
        process_path(path);
      }
    }
  }
}

#endif  /* HAVE_WINDOWS */
