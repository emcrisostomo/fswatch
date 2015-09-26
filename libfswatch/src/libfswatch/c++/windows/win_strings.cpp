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
#include "win_strings.hpp"
#include <windows.h>

namespace fsw
{
  namespace win_strings
  {
    using namespace std;

    string wstring_to_string(wchar_t * s)
    {
      int buf_size = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
      char buf[buf_size];
      WideCharToMultiByte(CP_UTF8, 0, s, -1, buf, buf_size, NULL, NULL);

      return string(buf);
    }

    string wstring_to_string(const wstring & s)
    {
      return wstring_to_string((wchar_t *)s.c_str());
    }
  }
}
