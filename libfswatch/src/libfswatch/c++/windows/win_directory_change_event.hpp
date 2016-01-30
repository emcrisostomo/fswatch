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
/**
 * @file
 * @brief Header of the fsw::directory_change_event class.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */
#ifndef FSW_WIN_DIRECTORY_CHANGE_EVENT_H
#  define	FSW_WIN_DIRECTORY_CHANGE_EVENT_H

#  include <cstdlib>
#  include <string>
#  include <memory>
#  include <vector>
#  include <windows.h>
#  include "win_handle.hpp"
#  include "win_error_message.hpp"
#  include "../event.hpp"

namespace fsw
{
  /**
   * @brief Header of the fsw::directory_change_event class, a helper class to
   * wrap Microsoft Windows' `ReadDirectoryChangesW` function and a common
   * workflow to detect file system changes.
   */
  class directory_change_event
  {
  public:
    std::wstring path;
    win_handle handle;
    size_t buffer_size;
    DWORD bytes_returned;
    std::unique_ptr<void, decltype(free)*> buffer = {nullptr, free};
    std::unique_ptr<OVERLAPPED, decltype(free)*> overlapped = {static_cast<OVERLAPPED *> (malloc(sizeof (OVERLAPPED))), free};
    win_error_message read_error;

    directory_change_event(size_t buffer_length = 16);
    bool is_io_incomplete();
    bool is_buffer_overflowed();
    bool read_changes_async();
    bool try_read();
    void continue_read();
    std::vector<event> get_events();
  };
}

#endif	/* WIN_DIRECTORY_CHANGE_EVENT_H */

