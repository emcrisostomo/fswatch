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
#include <cstdlib>
#include <iostream>

#ifdef _MSC_VER
#include "libfswatch_config.h"
#include <intrin.h>
#include <fileapi.h>
#define INVALID_HANDLE_VALUE (HANDLE)(-1)
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif /* ! PATH_MAX */
#else
#include <dirent.h>
#include <cstdio>
#include <cerrno>
#include <system_error>
#endif /* _MSC_VER */

using namespace std;

namespace fsw
{
  vector<string> get_directory_children(const string& path)
  {
   vector<string> children;
#ifdef _MSC_VER
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;

    if((hFind = FindFirstFile(path.c_str(), &FindFileData)) != INVALID_HANDLE_VALUE){
      do {
          children.emplace_back(FindFileData.cFileName);
      } while(FindNextFile(hFind, &FindFileData));
      FindClose(hFind);
    }
#else
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
#endif /* _MSC_VER */

    return children;
  }

  bool read_link_path(const string& path, string& link_path)
  {
    link_path = fsw_realpath(path.c_str(), nullptr);

    return true;
  }

  std::string fsw_realpath(const char *path, char *resolved_path)
  {
#ifdef _MSC_VER
    char *path_buf = /* _strdup(path) */ (char*)path;
    char *ret = _fullpath(path_buf, resolved_path, PATH_MAX);
    if (ret == NULL)
        throw std::exception("_fullpath", EXIT_FAILURE);

    return std::string(ret);
#else
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
#endif /* _MSC_VER */
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
#ifdef _MSC_VER
      return stat_path(path, fd_stat);
#else
      if (lstat(path.c_str(), &fd_stat) == 0)
      return true;
#endif /* _MSC_VER */
    fsw_logf_perror(_("Cannot lstat %s"), path.c_str());
    return false;
  }
}
