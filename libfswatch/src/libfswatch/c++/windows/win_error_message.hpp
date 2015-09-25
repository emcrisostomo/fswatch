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
#ifndef FSW_WINDOWS_ERROR_MESSAGE_H
#  define FSW_WINDOWS_ERROR_MESSAGE_H

#  include <string>
#  include <windows.h>

namespace fsw
{
  class win_error_message
  {
  public:
    static win_error_message current();

    win_error_message(DWORD err_code);
    win_error_message();

    DWORD get_error_code() const;

    std::wstring get_message() const;

    operator std::wstring() const;

  private:
    mutable bool initialized = false;
    mutable std::wstring msg;
    DWORD err_code;
  };
}

#endif  /* FSW_WINDOWS_ERROR_MESSAGE_H */
