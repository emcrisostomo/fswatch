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
 * @brief Header of the fsw::win_error_message class.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */
#ifndef FSW_WINDOWS_ERROR_MESSAGE_H
#  define FSW_WINDOWS_ERROR_MESSAGE_H

#  include <string>
#  include <windows.h>

namespace fsw
{
  /**
   * @brief Helper class to get the system-defined error message for a Microsoft
   * Windows' error code.
   *
   * This class uses the `FormatMessage()` API to returns a std::wstring
   * instance containing the system-defined error message for a Microsoft
   * Windows' error code.
   */
  class win_error_message
  {
  public:
    /**
     * @brief Constructs an instance of this class using the last error code of
     * the calling thread, returned by a call to `GetLastError()`.
     *
     * @see win_error_message()
     */
    static win_error_message current();

    /**
     * @brief Constructs an error message using the specified @p error_code.
     *
     * @param error_code The error code.
     */
    win_error_message(DWORD error_code);

    /**
     * @brief Constructs an error message using the last error code of the
     * calling thread, retrieved with a call to `GetLastError()`.
     *
     * @see current()
     */
    win_error_message();

    /**
     * @brief Gets the error code.
     *
     * @return The error code.
     */
    DWORD get_error_code() const;

    /**
     * @brief Gets the system-defined error message.
     *
     * The system-defined error message is retrieved with a call to
     * `FormatMessage` with the `FORMAT_MESSAGE_FROM_SYSTEM` formatting option.
     *
     * @return The error message.
     */
    std::wstring get_message() const;

    /**
     * @brief Gets ths system-defined error message.
     *
     * @see get_message()
     */
    operator std::wstring() const;

  private:
    mutable bool initialized = false;
    mutable std::wstring msg;
    DWORD err_code;
  };
}

#endif  /* FSW_WINDOWS_ERROR_MESSAGE_H */
