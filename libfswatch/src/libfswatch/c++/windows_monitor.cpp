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

using namespace std;

namespace fsw
{
  REGISTER_MONITOR_IMPL(windows_monitor, windows_monitor_type);

  class win_error_message
  {
  public:
    static win_error_message current()
    {
      win_error_message current;
      return std::move(current);
    }

    win_error_message(DWORD err_code) : err_code{err_code}{}
    win_error_message() : err_code{GetLastError()}{}

    wstring get_message() const
    {
      if (initialized) return msg;
      initialized = true;

      LPWSTR buf = nullptr;
      DWORD ret_size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                      NULL,
                                      err_code,
                                      0,
                                      (LPWSTR)&buf,
                                      0,
                                      nullptr);

      if (ret_size > 0)
      {
        msg = buf;
        LocalFree(buf);
      }
      else
      {
        msg = L"The system error message could not be formatted.";
      }

      return msg;
    }

    operator wstring() const { return get_message(); }

  private:
    mutable bool initialized = false;
    mutable wstring msg;
    DWORD err_code;
  };

  class win_handle
  {
  public:
    static bool is_valid(const HANDLE & handle)
    {
      return (handle != INVALID_HANDLE_VALUE && handle != nullptr);
    }

    win_handle() : h(INVALID_HANDLE_VALUE){}
    win_handle(HANDLE handle) : h(handle){}

    ~win_handle()
    {
      if (is_valid())
      {
        libfsw_logv(_("win_handle::~win_handle(): Closing handle: %d.\n"), h);
        CloseHandle(h);
      }
    }

    operator HANDLE() const { return h; }

    bool is_valid() const
    {
      return win_handle::is_valid(h);
    }

    win_handle(const win_handle&) = delete;
    win_handle& operator=(const win_handle&) = delete;

    win_handle& operator=(const HANDLE& handle)
    {
      if (is_valid()) ::CloseHandle(h);

      h = handle;

      return *this;
    }

    win_handle(win_handle&& other) noexcept
    {
      h = other.h;
      other.h = INVALID_HANDLE_VALUE;
    }

    win_handle& operator=(win_handle&& other) noexcept
    {
      if (this == &other) return *this;

      if (is_valid()) ::CloseHandle(h);

      h = other.h;
      other.h = INVALID_HANDLE_VALUE;

      return *this;
    }

  private:
    HANDLE h;
  };

  typedef struct directory_change_event
  {
    win_handle handle;
    size_t buffer_size;
    DWORD bytes_returned;
    unique_ptr<void, decltype(::free)*> buffer = {nullptr, ::free};
    unique_ptr<OVERLAPPED, decltype(::free)*> overlapped = {static_cast<OVERLAPPED *>(::malloc(sizeof(OVERLAPPED))), ::free};

    directory_change_event(size_t buffer_length = 16) : handle{INVALID_HANDLE_VALUE},
                                                        buffer_size{sizeof(FILE_NOTIFY_INFORMATION) * buffer_length},
                                                        bytes_returned{}
    {
      buffer.reset(::malloc(buffer_size));
      if (buffer.get() == nullptr) throw libfsw_exception(_("::malloc failed."));
      if (overlapped.get() == nullptr) throw libfsw_exception(_("::malloc failed."));
    }
  } directory_change_event;

  struct windows_monitor_load
  {
    fsw_hash_set<wstring> win_paths;
    fsw_hash_map<wstring, directory_change_event> dce_by_path;
    fsw_hash_map<wstring, win_handle> event_by_path;
  };

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

  windows_monitor::windows_monitor(vector<string> paths_to_monitor,
                                   FSW_EVENT_CALLBACK * callback,
                                   void * context) :
    monitor(paths_to_monitor, callback, context), load(new windows_monitor_load())
  {
  }

  windows_monitor::~windows_monitor()
  {
    delete load;
  }

  void windows_monitor::initialize_windows_path_list()
  {
    for (const auto & path : paths)
    {
      void * raw_path = ::cygwin_create_path(CCP_POSIX_TO_WIN_W, path.c_str());
      if (raw_path == nullptr) throw libfsw_exception(_("cygwin_create_path could not allocate memory."));

      load->win_paths.insert(wstring(static_cast<wchar_t *>(raw_path)));

      ::free(raw_path);
    }
  }

  static bool read_directory_changes(directory_change_event & dce)
  {
    libfsw_logv(_("read_directory_changes: %p.\n"), &dce);

    return ReadDirectoryChangesW((HANDLE)dce.handle,
                                 dce.buffer.get(),
                                 dce.buffer_size,
                                 TRUE,
                                 FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                 FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION,
                                 &dce.bytes_returned,
                                 dce.overlapped.get(),
                                 nullptr);
  }

  void windows_monitor::initialize_events()
  {
    for (const wstring & path : load->win_paths)
    {
      libfsw_logv(_("initialize_events: creating event for %s.\n"), wstring_to_string(path).c_str());

      HANDLE h = ::CreateEvent(nullptr,
                               TRUE,
                               FALSE,
                               nullptr);

      if (h == NULL) throw libfsw_exception(_("CreateEvent failed."));

      libfsw_logv(_("initialize_events: event %d created for %s.\n"), h, wstring_to_string(path).c_str());

      load->event_by_path.emplace(path, h);
    }
  }

  bool windows_monitor::init_search_for_path(const wstring path)
  {
    libfsw_logv(_("init_search_for_path: %s.\n"), wstring_to_string(path).c_str());

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

    libfsw_logv(_("init_search_for_path: file handle: %d.\n"), h);

    directory_change_event dce(128);
    dce.handle = h;
    dce.overlapped.get()->hEvent = load->event_by_path[path];

    if (!read_directory_changes(dce))
    {
      libfsw_logv("ReadDirectoryChangesW: %s\n", wstring_to_string(win_error_message::current()).c_str());
      return false;
    }

    load->dce_by_path[path] = std::move(dce);

    return true;
  }

  void windows_monitor::stop_search_for_path(const wstring path)
  {
    load->dce_by_path.erase(path);
  }

  void windows_monitor::run()
  {
    // Since the file handles are open with FILE_SHARE_DELETE, it may happen
    // that file is deleted when a handle to it is being used.  A call to
    // either ReadDirectoryChangesW or GetOverlappedResult will return with
    // an error if the file system object being observed is deleted.
    // Unfortunately, the error reported by Windows is `Access denied',
    // preventing fswatch to report better messages to the user.

    SetConsoleOutputCP(CP_UTF8);

    initialize_windows_path_list();
    initialize_events();

    while (true)
    {
      ::sleep(latency);

      for (const auto & path : load->win_paths)
      {
        libfsw_logv(_("run: processing %s.\n"), wstring_to_string(path).c_str());

        // If the path is not currently watched, then initialize the search
        // structures.  If the initalization fails, skip the path altogether
        // until the next iteration.
        auto it = load->dce_by_path.find(path);
        if (it == load->dce_by_path.end())
        {
          libfsw_logv(_("run: initializing search structures for %s.\n"), wstring_to_string(path).c_str());
          if (!init_search_for_path(path)) continue;
        }

        it = load->dce_by_path.find(path);
        if (it == load->dce_by_path.end()) throw libfsw_exception(_("Initialization failed."));

        directory_change_event & dce = it->second;

        if(!GetOverlappedResult(dce.handle, dce.overlapped.get(), &dce.bytes_returned, FALSE))
        {
          DWORD err = GetLastError();
          if (err == ERROR_IO_INCOMPLETE)
          {
            libfsw_logv(_("run: I/O incomplete.\n"));
            continue;
          }
          else if (err == ERROR_NOTIFY_ENUM_DIR)
          {
            cerr << "Overflow." << endl;
          }

          // TODO: this error should be logged only in verbose mode.
          fprintf(stderr, "GetOverlappedResult: %s\n", wstring_to_string((wstring)win_error_message(err)).c_str());
          stop_search_for_path(path);
          continue;
        }

        libfsw_logv(_("run: GetOverlappedResult returned %d bytes\n"), dce.bytes_returned);

        if(dce.bytes_returned == 0)
        {
          throw libfsw_exception(_("Event queue overflow."));
        }
        else
        {
          time_t curr_time;
          time(&curr_time);
          vector<event> events;

          char * curr_entry = static_cast<char *>(dce.buffer.get());

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
              string file_name = wstring_to_string(path)
                + "\\"
                + wstring_to_string(wstring(currEntry->FileName, currEntry->FileNameLength/sizeof(wchar_t)));

              events.push_back({file_name, curr_time, decode_flags(currEntry->Action)});
            }
            else
            {
              cerr << _("File name unexpectedly empty.") << endl;
            }

            curr_entry = (currEntry->NextEntryOffset == 0) ? nullptr : curr_entry + currEntry->NextEntryOffset;
          }

          if (events.size()) notify_events(events);
        }

        if (!ResetEvent(dce.overlapped.get()->hEvent)) throw libfsw_exception(_("::ResetEvent failed."));
        else libfsw_logv(_("run: event %d reset.\n"), dce.overlapped.get()->hEvent);

        if (!read_directory_changes(dce))
        {
          libfsw_logv("ReadDirectoryChangesW: %s\n", wstring_to_string(win_error_message::current()).c_str());
          stop_search_for_path(path);
          continue;
        }
      }
    }
  }
}

#endif  /* HAVE_WINDOWS */
