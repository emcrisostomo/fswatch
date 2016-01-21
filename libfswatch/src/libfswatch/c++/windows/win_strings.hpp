/*
 * Copyright (c) 2016 Enrico M. Crisostomo
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
 * @brief Header of the fsw::win_strings namespace.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef FSW_WIN_STRINGS_H
#  define  FSW_WIN_STRINGS_H

#  include <string>
#  include <cwchar>

namespace fsw
{
  /**
   * @brief String conversion functions.
   *
   * This namespace contains utility functions to convert wide character strings
   * into strings.
   */
  namespace win_strings
  {
    /**
     * @brief Converts a wide character string into a string.
     *
     * @param s The @c wchar_t array to convert.
     * @return The converted string.
     */
    std::string wstring_to_string(wchar_t *s);

    /**
     * @brief Converts a wide character string into a string.
     *
     * @param s The string to convert.
     * @return The converted string.
     */
    std::string wstring_to_string(const std::wstring& s);
  }
}

#endif	/* FSW_WIN_STRINGS_H */
