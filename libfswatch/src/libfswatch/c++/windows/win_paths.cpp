/*
 * Copyright (c) 2015-2025 Enrico M. Crisostomo
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
#include "win_paths.hpp"
#include <windows.h>
#include <vector>
#include "../libfswatch_exception.hpp"
#include "../../gettext_defs.h"

namespace fsw
{
  namespace win_paths
  {
    std::wstring posix_to_win_w(std::string path)
    {
      int pathlen = (int) path.length() + 1;
      int buflen = MultiByteToWideChar(CP_ACP, 0, path.c_str(), pathlen, 0, 0);
      std::vector<wchar_t> buf(buflen);
      MultiByteToWideChar(CP_ACP, 0, path.c_str(), pathlen, buf.data(), buflen);
      return std::wstring(buf.data());
    }

    std::string win_w_to_posix(std::wstring path)
    {
      int pathlen = (int)path.length() + 1;
      int buflen = WideCharToMultiByte(CP_ACP, 0, path.c_str(), pathlen, 0, 0, 0, 0);
      std::vector<char> buf(buflen);
      WideCharToMultiByte(CP_ACP, 0, path.c_str(), pathlen, buf.data(), buflen, 0, 0);
      return std::string(buf.data());
    }
  }
}
