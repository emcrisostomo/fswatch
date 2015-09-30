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
#include <sys/cygwin.h>
#include "../libfswatch_exception.hpp"
#include "../../gettext_defs.h"

using namespace std;

namespace fsw
{
  namespace win_paths
  {
    wstring posix_to_win_w(string path)
    {
      void * raw_path = cygwin_create_path(CCP_POSIX_TO_WIN_W, path.c_str());
      if (raw_path == nullptr) throw libfsw_exception(_("cygwin_create_path could not allocate memory to convert the path."));

      wstring win_path(static_cast<wchar_t *> (raw_path));

      free(raw_path);

      return win_path;
    }

    string win_w_to_posix(wstring path)
    {
      void * raw_path = cygwin_create_path(CCP_WIN_W_TO_POSIX, path.c_str());
      if (raw_path == nullptr) throw libfsw_exception(_("cygwin_create_path could not allocate memory to convert the path."));

      string posix_path(static_cast<char *> (raw_path));

      free(raw_path);

      return posix_path;
    }
  }
}
