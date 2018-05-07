/*
 * Copyright (c) 2014-2018 Enrico M. Crisostomo
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
#include "gettext_defs.h"
#include "path_utils.hpp"
#include "c/libfswatch_log.h"
#include <dirent.h>
#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <iostream>

using namespace std;

namespace fsw
{
  vector<string> get_directory_children(const string& path)
  {
    vector<string> children;
    DIR *dir = opendir(path.c_str());

    if (!dir)
    {
      if (errno == EMFILE || errno == ENFILE)
      {
        perror("opendir");
      }
      else
      {
        fsw_log_perror("opendir");
      }

      return children;
    }

    while (struct dirent *ent = readdir(dir))
    {
      children.emplace_back(ent->d_name);
    }

    closedir(dir);

    return children;
  }

  bool read_link_path(const string& path, string& link_path)
  {
    char *real_path = realpath(path.c_str(), nullptr);
    link_path = (real_path ? real_path : path);

    bool ret = (real_path != nullptr);
    free(real_path);

    return ret;
  }

  bool stat_path(const string& path, struct stat& fd_stat)
  {
    if (stat(path.c_str(), &fd_stat) != 0)
    {
      fsw_logf_perror(_("Cannot stat %s"), path.c_str());

      return false;
    }

    return true;
  }

  bool lstat_path(const string& path, struct stat& fd_stat)
  {
    if (lstat(path.c_str(), &fd_stat) != 0)
    {
      fsw_logf_perror(_("Cannot lstat %s"), path.c_str());

      return false;
    }

    return true;
  }
}
