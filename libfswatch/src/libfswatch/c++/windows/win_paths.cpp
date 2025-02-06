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
#include "win_paths.hpp"
#ifdef __CYGWIN__
#include <sys/cygwin.h>
#else
#include <windows.h>
#include <vector>
#endif
#include "../libfswatch_exception.hpp"
#include "../../gettext_defs.h"

using namespace std;

namespace fsw
{
  namespace win_paths
  {
    wstring posix_to_win_w(string path)
    {
#ifdef __CYGWIN__
      void * raw_path = cygwin_create_path(CCP_POSIX_TO_WIN_W, path.c_str());
      if (raw_path == nullptr) throw libfsw_exception(_("cygwin_create_path could not allocate memory to convert the path."));

      wstring win_path(static_cast<wchar_t *> (raw_path));

      free(raw_path);

      return win_path;
#else
      int pathlen = (int) path.length() + 1;
      int buflen = MultiByteToWideChar(CP_ACP, 0, path.c_str(), pathlen, 0, 0);
      std::vector<wchar_t> buf(buflen);
      MultiByteToWideChar(CP_ACP, 0, path.c_str(), pathlen, buf.data(), buflen);
      return std::wstring(buf.data());
#endif
    }

    string win_w_to_posix(wstring path)
    {
#ifdef __CYGWIN__
      void * raw_path = cygwin_create_path(CCP_WIN_W_TO_POSIX, path.c_str());
      if (raw_path == nullptr) throw libfsw_exception(_("cygwin_create_path could not allocate memory to convert the path."));

      string posix_path(static_cast<char *> (raw_path));

      free(raw_path);

      return posix_path;
#else
      int pathlen = (int)path.length() + 1;
      int buflen = WideCharToMultiByte(CP_ACP, 0, path.c_str(), pathlen, 0, 0, 0, 0);
      std::vector<char> buf(buflen);
      WideCharToMultiByte(CP_ACP, 0, path.c_str(), pathlen, buf.data(), buflen, 0, 0);
      return std::string(buf.data());
#endif
    }
  }
}
