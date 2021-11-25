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
#include <stdlib.h>
#include <locale>
#include "win_paths.hpp"
#include "../libfswatch_exception.hpp"
#include "../../gettext_defs.h"

namespace fsw
{
  namespace win_paths
  {
    std::wstring posix_to_win_w(std::string path)
    {
      std::wstring ws = std::make_unique<wchar_t[]>(path.size() + 1);
      mbstowcs_s(nullptr, ws.get(), path.size() + 1, path.c_str(), path.size());
      return ws;
    }

    std::string win_w_to_posix(std::wstring path)
    {
      std::string str(path.length(), 0);
      std::transform(path.begin(), path.end(), str.begin(), [] (wchar_t c) {
          return (char)c;
      });
      return str;
    }
  }
}
