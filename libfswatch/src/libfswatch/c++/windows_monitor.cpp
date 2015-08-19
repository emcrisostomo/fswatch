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
        msg = L"Cannot format system error message.";
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
      return (handle != INVALID_HANDLE_VALUE);
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
    DWORD nBufferLength;
    size_t bufferSize;
    DWORD bytesReturned;
    unique_ptr<void, decltype(::free)*> lpBuffer = {nullptr, ::free};
    OVERLAPPED overlapped;

    DirectoryChangeEvent() : handle(INVALID_HANDLE_VALUE), nBufferLength(16),
                             bufferSize(sizeof(FILE_NOTIFY_INFORMATION) * nBufferLength),
                             bytesReturned(), overlapped()
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
      dce.overlapped.hEvent = CreateEvent(nullptr,
                                            TRUE,
                                            FALSE,
                                            nullptr);

      if (dce.overlapped.hEvent == NULL) throw libfsw_exception(_("CreateEvent failed."));

      load->dce_by_path[path] = std::move(dce);
    }
  }

  
  void windows_monitor::run()
  {
    initialize_windows_path_list();
    initial_scan();

    while (true)
    {
      for (auto path_dce_pair = load->dce_by_path.begin(); path_dce_pair != load->dce_by_path.end(); path_dce_pair++)
      {
        while (true)
        {
          DirectoryChangeEvent & dce = path_dce_pair->second;

          BOOL b = ReadDirectoryChangesW((HANDLE)dce.handle,
                                         dce.lpBuffer.get(),
                                         dce.bufferSize,
                                         TRUE,
                                         FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                         FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION,
                                         &dce.bytesReturned,
                                         &dce.overlapped,
                                         nullptr);

          if (!b) throw libfsw_exception(_("ReadDirectoryChangesW failed."));

          BOOL res = GetOverlappedResult(dce.handle, &dce.overlapped, &dce.bytesReturned, TRUE);
          if (!res || dce.bytesReturned == 0)
          {
            WinErrorMessage error;
            wcerr << (wstring)error << endl;
            continue;
          }

          FILE_NOTIFY_INFORMATION * currEntry = static_cast<FILE_NOTIFY_INFORMATION *>(dce.lpBuffer.get());

          while (currEntry != nullptr)
          {
            if (currEntry->FileNameLength > 0)
            {
              wchar_t * stringBuffer = static_cast<wchar_t *>(LocalAlloc(LMEM_ZEROINIT, currEntry->FileNameLength + sizeof(wchar_t)));
              if (stringBuffer == nullptr) throw libfsw_exception(_("::LocalAlloc failed."));
              memcpy(stringBuffer, currEntry->FileName, currEntry->FileNameLength);
              wcout << stringBuffer << endl;
              LocalFree(stringBuffer);
            }
            currEntry = (currEntry->NextEntryOffset == 0) ? nullptr : currEntry + currEntry->NextEntryOffset;
          }
        }
      }
    }
  }
}

#endif  /* HAVE_WINDOWS */
