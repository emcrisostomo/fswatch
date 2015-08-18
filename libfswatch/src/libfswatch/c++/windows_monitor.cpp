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
  struct windows_monitor_load
  {
    fsw_hash_set<wstring> win_paths;

    fsw_hash_map<wstring, int> descriptors_by_file_name;
    fsw_hash_map<int, string> file_names_by_descriptor;
    fsw_hash_set<int> descriptors_to_remove;
    fsw_hash_set<int> descriptors_to_rescan;
    fsw_hash_map<int, mode_t> file_modes;
  };

  REGISTER_MONITOR_IMPL(windows_monitor, windows_monitor_type);

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

  void windows_monitor::run()
  {
    initialize_windows_path_list();

    const DWORD nBufferLength = 16;
    DWORD bytesReturned;
    auto lpBuffer = unique_ptr<void, decltype(::free)*> {
        ::malloc(sizeof(FILE_NOTIFY_INFORMATION) * nBufferLength),
        ::free
      };
    if (lpBuffer.get() == nullptr) throw libfsw_exception(_("::malloc failed."));

    while (true)
    {
      for (const auto & path : load->win_paths)
      {
        HANDLE h = CreateFileW(path.c_str(),
                               GENERIC_READ,
                               FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                               nullptr, OPEN_EXISTING,
                               FILE_FLAG_BACKUP_SEMANTICS,
                               nullptr);
        if (h == INVALID_HANDLE_VALUE)
        {
          wcerr << L"Invalid handle when opening " << path << endl;
          continue;
        }

        BOOL b = ReadDirectoryChangesW(h,
                                       lpBuffer.get(),
                                       nBufferLength * sizeof(FILE_NOTIFY_INFORMATION),
                                       TRUE,
                                       FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                       FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION,
                                       &bytesReturned,
                                       nullptr,
                                       nullptr);

        if (!b) throw libfsw_exception(_("ReadDirectoryChangesW failed."));
        if (bytesReturned == 0)
        {
          cout << "Emtpy result set." << endl;
          LPWSTR pTemp = nullptr;
          DWORD retSize = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                         NULL,
                                         GetLastError(),
                                         LANG_NEUTRAL,
                                         (LPWSTR)&pTemp,
                                         0,
                                         nullptr);

          if (retSize > 0)
          {
            wcerr << pTemp << endl;
            LocalFree(pTemp);
          }

          continue;
        }

        FILE_NOTIFY_INFORMATION * currEntry = static_cast<FILE_NOTIFY_INFORMATION *>(lpBuffer.get());

        while (currEntry != nullptr)
        {
          cout << "Next entry offset " << currEntry->NextEntryOffset << endl;
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

#endif  /* HAVE_WINDOWS */
