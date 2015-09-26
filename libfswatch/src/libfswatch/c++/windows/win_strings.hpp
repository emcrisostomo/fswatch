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

#ifndef FSW_WIN_STRINGS_H
#  define	FSW_WIN_STRINGS_H

#  include <string>
#  include <cwchar>

namespace fsw
{
  namespace win_strings
  {
    static std::string wstring_to_string(wchar_t * s);
    static std::string wstring_to_string(const std::wstring & s);
  }
}

#endif	/* FSW_WIN_STRINGS_H */
