/* 
 * Copyright (C) 2014, Enrico M. Crisostomo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "gettext_defs.h"
#include "path_utils.h"
#include "c/libfswatch_log.h"
#include <dirent.h>
#include <cstdlib>
#include <errno.h>
#include <iostream>

using namespace std;

namespace fsw
{

  void get_directory_children(const string &path, vector<string> &children)
  {
    DIR *dir = ::opendir(path.c_str());

    if (!dir)
    {
      if (errno == EMFILE || errno == ENFILE)
      {
        perror("opendir");
        // ::exit(FSW_EXIT_ENFILE);
      }
      else
      {
        libfsw_perror("opendir");
      }

      return;
    }

    while (struct dirent * ent = readdir(dir))
    {
      children.push_back(ent->d_name);
    }

    ::closedir(dir);
  }

  bool read_link_path(const string &path, string &link_path)
  {
    char *real_path = ::realpath(path.c_str(), nullptr);
    link_path = (real_path ? real_path : path);

    bool ret = (real_path != nullptr);
    ::free(real_path);

    return ret;
  }

  bool stat_path(const string &path, struct stat &fd_stat)
  {
    if (::lstat(path.c_str(), &fd_stat) != 0)
    {
      string err = string(_("Cannot stat() ")) + path;
      libfsw_perror(err.c_str());

      return false;
    }

    return true;
  }
}
