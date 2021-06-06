/*
 * Copyright (c) 2014-2021 Enrico M. Crisostomo
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
#include "libfswatch/gettext_defs.h"
#include "path_utils.hpp"
#include "libfswatch/c/libfswatch_log.h"
#include <dirent.h>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <iostream>
#include <system_error>

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
    link_path = fsw_realpath(path.c_str(), nullptr);

    return true;
  }

  std::string fsw_realpath(const char *path, char *resolved_path)
  {
    char *ret = realpath(path, resolved_path);

    if (ret == nullptr)
    {
      if (errno != ENOENT)
        throw std::system_error(errno, std::generic_category());

      return std::string(path);
    }

    std::string resolved(ret);

    if (resolved_path == nullptr) free(ret);

    return resolved;
  }

  bool stat_path(const string& path, struct stat& fd_stat)
  {
    if (stat(path.c_str(), &fd_stat) == 0)
      return true;

    fsw_logf_perror(_("Cannot stat %s"), path.c_str());
    return false;

  }

  bool lstat_path(const string& path, struct stat& fd_stat)
  {
    if (lstat(path.c_str(), &fd_stat) == 0)
      return true;

    fsw_logf_perror(_("Cannot lstat %s"), path.c_str());
    return false;
  }
}
