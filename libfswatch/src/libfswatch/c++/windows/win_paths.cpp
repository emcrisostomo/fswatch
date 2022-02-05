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
#include <algorithm>
#include <codecvt>
#include <locale>
#include <memory>
#include "win_paths.hpp"

namespace fsw
{
  namespace win_paths
  {
    std::wstring posix_to_win_w(const std::string &path)
    {
      std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
      return converter.from_bytes(path.c_str());
    }

    std::string win_w_to_posix(const std::wstring &path)
    {
      /*
      std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
      return converter.to_bytes(path);
      */
      std::string str(path.length(), 0);
      std::transform(path.begin(), path.end(), str.begin(), [] (wchar_t c) {
          return (char)c;
      });
      return str;
    }
  }
}
