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
#  include "path_utils.hpp"
#  include <iostream>
#  include <memory>
#  include <sys/types.h>
#  include <cstdlib>
#  include <cstring>
#  include <ctime>
#  include <cstdio>
#  include <cmath>
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/cygwin.h>
#  include <windows.h>

using namespace std;

namespace fsw
{
  REGISTER_MONITOR_IMPL(windows_monitor, windows_monitor_type);

  class WinErrorMessage
  {
  public:
    static WinErrorMessage current()
    {
      WinErrorMessage current;
      return std::move(current);
    }

    WinErrorMessage(DWORD errCode) : errCode{errCode}{}
    WinErrorMessage() : errCode{GetLastError()}{}

    wstring get_message() const
    {
      if (initialized) return msg;
      initialized = true;

      LPWSTR pTemp = nullptr;
      DWORD retSize = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL,
                                     errCode,
                                     0,
                                     (LPWSTR)&pTemp,
                                     0,
                                     nullptr);

      if (retSize > 0)
      {
        msg = pTemp;
        LocalFree(pTemp);
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
    DWORD errCode;
  };

  class CHandle
  {
  public:
    static bool is_valid(const HANDLE & handle)
    {
      return (handle != INVALID_HANDLE_VALUE && handle != nullptr);
    }

    CHandle() : h(INVALID_HANDLE_VALUE){}
    CHandle(HANDLE handle) : h(handle){}

    ~CHandle()
    {
      if (is_valid())
      {
        ::CloseHandle(h);
      }
    }

    operator HANDLE() const { return h; }

    bool is_valid() const
    {
      return CHandle::is_valid(h);
    }

    CHandle(const CHandle&) = delete;
    CHandle& operator=(const CHandle&) = delete;

    CHandle& operator=(const HANDLE& handle)
    {
      if (is_valid()) ::CloseHandle(h);

      h = handle;

      return *this;
    }

    CHandle(CHandle&& other)
    {
      h = other.h;
      other.h = INVALID_HANDLE_VALUE;
    }

    CHandle& operator=(CHandle&& other)
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

  typedef struct DirectoryChangeEvent
  {
    CHandle handle;
    size_t bufferSize;
    DWORD bytesReturned;
    unique_ptr<void, decltype(::free)*> lpBuffer = {nullptr, ::free};
    OVERLAPPED overlapped;

    DirectoryChangeEvent(size_t buffer_length = 16) : handle{INVALID_HANDLE_VALUE},
                                                      bufferSize{sizeof(FILE_NOTIFY_INFORMATION) * buffer_length},
                                                      bytesReturned{},
                                                      overlapped{}
    {
      lpBuffer.reset(::malloc(bufferSize));
      if (lpBuffer.get() == nullptr) throw libfsw_exception(_("::malloc failed."));
    }
  } DirectoryChangeEvent;

  struct windows_monitor_load
  {
    fsw_hash_set<wstring> win_paths;
    fsw_hash_map<wstring, DirectoryChangeEvent> dce_by_path;
    fsw_hash_map<wstring, int> descriptors_by_file_name;
    fsw_hash_map<int, string> file_names_by_descriptor;
    fsw_hash_set<int> descriptors_to_remove;
    fsw_hash_set<int> descriptors_to_rescan;
    fsw_hash_map<int, mode_t> file_modes;
  };

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

  bool windows_monitor::is_path_watched(const wstring path) const
  {
    return load->descriptors_by_file_name.find(path) != load->descriptors_by_file_name.end();
  }

  void windows_monitor::initialize_windows_path_list()
  {
    for (const auto & path : paths)
    {
      void * raw_path = cygwin_create_path(CCP_POSIX_TO_WIN_W, path.c_str());
      if (raw_path == nullptr) throw libfsw_exception(_("cygwin_create_path could not allocate memory."));

      load->win_paths.insert(wstring(static_cast<wchar_t *>(raw_path)));

      ::free(raw_path);
    }
  }

  void windows_monitor::initial_scan()
  {
    for (const auto & path : load->win_paths)
    {
      HANDLE h = CreateFileW(path.c_str(),
                             GENERIC_READ,
                             FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                             nullptr, OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                             nullptr);

      if (!CHandle::is_valid(h))
      {
        // To do: format error message
        wcerr << L"Invalid handle when opening " << path << endl;
        continue;
      }

      DirectoryChangeEvent dce;
      dce.handle = h;

      load->dce_by_path[path] = std::move(dce);
    }
  }

  void windows_monitor::run()
  {
    initialize_windows_path_list();
    initial_scan();

    while (true)
    {
      ::sleep(latency);

      for (auto path_dce_pair = load->dce_by_path.begin(); path_dce_pair != load->dce_by_path.end(); )
      {
        DirectoryChangeEvent & dce = path_dce_pair->second;

        // Since the file handles are open with FILE_SHARE_DELETE, it may
        // happen that file is deleted when a handle to it is being used.
        // A blocking call to GetOverlappedResult will return with an error
        // if the file system object being observed is deleted.  Unfortunately,
        // the error reported by Windows is `Access denied', preventing
        // fswatch to report better messages to the user.
        // if(dce.overlapped.Internal != STATUS_PENDING &&

        if (!CHandle::is_valid(dce.overlapped.hEvent))
        {
          dce.overlapped = OVERLAPPED{};
          dce.overlapped.hEvent = CreateEvent(nullptr,
                                              TRUE,
                                              FALSE,
                                              nullptr);

          if (dce.overlapped.hEvent == NULL) throw libfsw_exception(_("CreateEvent failed."));

          if (!ReadDirectoryChangesW((HANDLE)dce.handle,
                                     dce.lpBuffer.get(),
                                     dce.bufferSize,
                                     TRUE,
                                     FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                     FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION,
                                     &dce.bytesReturned,
                                     &dce.overlapped,
                                     nullptr))
          {
            // TODO: this error should be logged only in verbose mode.
            wcout << L"ReadDirectoryChangesW: " << (wstring)WinErrorMessage::current() << endl;
            // load->dce_by_path.erase(path_dce_pair++);
            ++path_dce_pair;
            continue;
          }
        }

        if(!GetOverlappedResult(dce.handle, &dce.overlapped, &dce.bytesReturned, FALSE))
        {
          DWORD err = GetLastError();
          if (err == ERROR_IO_INCOMPLETE)
          {
            ++path_dce_pair;
            continue;
          }
          else if (err == ERROR_NOTIFY_ENUM_DIR)
          {
            cerr << "Overflow." << endl;
          }

          // TODO: this error should be logged only in verbose mode.
          wcout << L"GetOverlappedResult: " << (wstring)WinErrorMessage(err) << endl;
          load->dce_by_path.erase(path_dce_pair++);
          continue;
        }

        if(dce.bytesReturned == 0)
        {
          cerr << _("The current buffer is too small.") << endl;
        }
        else
        {
          char * curr_entry = static_cast<char *>(dce.lpBuffer.get());

          while (curr_entry != nullptr)
          {
            FILE_NOTIFY_INFORMATION * currEntry = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(curr_entry);

            if (currEntry->FileNameLength > 0)
            {
              wchar_t * stringBuffer = static_cast<wchar_t *>(LocalAlloc(LMEM_ZEROINIT, currEntry->FileNameLength + sizeof(wchar_t)));
              if (stringBuffer == nullptr) throw libfsw_exception(_("::LocalAlloc failed."));

              memcpy(stringBuffer, currEntry->FileName, currEntry->FileNameLength);
              wcout << stringBuffer << endl;

              LocalFree(stringBuffer);
            }

            curr_entry = (currEntry->NextEntryOffset == 0) ? nullptr : curr_entry + currEntry->NextEntryOffset;
          }
        }

        CloseHandle(dce.overlapped.hEvent);
        dce.overlapped.hEvent = nullptr;;

        ++path_dce_pair;
      }
    }
  }
}

#endif  /* HAVE_WINDOWS */
