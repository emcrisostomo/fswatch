/*
 * Copyright (c) 2015 Enrico M. Crisostomo
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

#include "win_directory_change_event.hpp"
#include "win_paths.hpp"
#include "win_strings.hpp"
#include "../libfswatch_exception.hpp"
#include "../../c/libfswatch_log.h"
#include "../../gettext_defs.h"
#include <string>
#include <cstdlib>
#include <set>

namespace fsw
{
  using namespace std;

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

  directory_change_event::directory_change_event(size_t buffer_length)
    : handle{INVALID_HANDLE_VALUE},
      buffer_size{sizeof (FILE_NOTIFY_INFORMATION) * buffer_length},
      bytes_returned{}
    {
      buffer.reset(malloc(buffer_size));
      if (buffer.get() == nullptr) throw libfsw_exception(_("malloc failed."));
      if (overlapped.get() == nullptr) throw libfsw_exception(_("malloc failed."));
    }

  bool directory_change_event::is_io_incomplete()
  {
    return (read_error.get_error_code() == ERROR_IO_INCOMPLETE);
  }

  bool directory_change_event::is_buffer_overflowed()
  {
    return (read_error.get_error_code() == ERROR_NOTIFY_ENUM_DIR);
  }

  bool directory_change_event::read_changes_async()
  {
    continue_read();

    FSW_ELOGF(_("%p.\n"), this);

    return ReadDirectoryChangesW((HANDLE) handle,
                                 buffer.get(),
                                 buffer_size,
                                 TRUE,
                                 FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                 FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION,
                                 &bytes_returned,
                                 overlapped.get(),
                                 nullptr);
  }

  bool directory_change_event::try_read()
  {
    bool ret = GetOverlappedResult(handle, overlapped.get(), &bytes_returned, FALSE);

    read_error = win_error_message::current();

    FSW_ELOGF(_("GetOverlappedResult: %s\n"), win_strings::wstring_to_string((wstring) read_error).c_str());

    return ret;
  }

  void directory_change_event::continue_read()
  {
    if (!ResetEvent(overlapped.get()->hEvent)) throw libfsw_exception(_("ResetEvent failed."));

    FSW_ELOGF(_("Event %d reset.\n"), overlapped.get()->hEvent);
  }

  vector<event> directory_change_event::get_events()
  {
    // TO DO: We are relying on callers to know events are ready.
    vector<event> events;

    time_t curr_time;
    time(&curr_time);

    char * curr_entry = static_cast<char *> (buffer.get());

    while (curr_entry != nullptr)
    {
      FILE_NOTIFY_INFORMATION * currEntry = reinterpret_cast<FILE_NOTIFY_INFORMATION *> (curr_entry);

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
          + wstring(currEntry->FileName, currEntry->FileNameLength / sizeof (wchar_t));

        events.push_back({win_paths::win_w_to_posix(file_name), curr_time, decode_flags(currEntry->Action)});
      }
      else
      {
        cerr << _("File name unexpectedly empty.") << endl;
      }

      curr_entry = (currEntry->NextEntryOffset == 0) ? nullptr : curr_entry + currEntry->NextEntryOffset;
    }

    return events;
  }
}
