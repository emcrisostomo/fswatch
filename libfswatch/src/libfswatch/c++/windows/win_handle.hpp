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
#ifndef FSW_WINDOWS_HANDLE_H
#  define FSW_WINDOWS_HANDLE_H

#  include <windows.h>

namespace fsw
{
  class win_handle
  {
  public:
    static bool is_valid(const HANDLE & handle);

    win_handle();
    win_handle(HANDLE handle);

    virtual ~win_handle();

    operator HANDLE() const;

    bool is_valid() const;

    win_handle(const win_handle&) = delete;
    win_handle& operator=(const win_handle&) = delete;

    win_handle(win_handle&& other) noexcept;
    win_handle& operator=(win_handle&& other) noexcept;

    win_handle& operator=(const HANDLE& handle);
  private:
    HANDLE h;
  };
}

#endif  /* FSW_WINDOWS_HANDLE_H */
