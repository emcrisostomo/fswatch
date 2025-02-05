/*
 * Copyright (c) 2014-2024 Enrico M. Crisostomo
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

namespace fsw
{
  std::vector<std::filesystem::directory_entry> get_directory_entries(const std::filesystem::path& path)
  {
    std::vector<std::filesystem::directory_entry> entries;
    // Reserve capacity to optimize memory allocation
    entries.reserve(std::distance(std::filesystem::directory_iterator(path), std::filesystem::directory_iterator{}));

    try
    {
      for (const auto& entry : std::filesystem::directory_iterator(path)) 
        entries.emplace_back(entry);
    } 
    catch (const std::filesystem::filesystem_error& e) 
    {
      FSW_ELOGF(_("Error accessing directory: %s"), e.what());
    }

    return entries;
  }
  
  std::vector<std::filesystem::directory_entry> get_subdirectories(const std::filesystem::path& path)
  {
    std::vector<std::filesystem::directory_entry> entries;
    // Reserve an initial capacity to reduce the number of reallocations
    entries.reserve(64);

    try
    {
      for (const auto& entry : std::filesystem::directory_iterator(path)) 
        if (entry.is_directory()) entries.emplace_back(entry);
    } 
    catch (const std::filesystem::filesystem_error& e) 
    {
      FSW_ELOGF(_("Error accessing directory: %s"), e.what());
    }

    return entries;
  }

  bool stat_path(const std::string& path, struct stat& fd_stat, bool follow_symlink)
  {
    return follow_symlink ? lstat_path(path, fd_stat) : stat_path(path, fd_stat);
  }

  bool stat_path(const std::string& path, struct stat& fd_stat)
  {
    if (stat(path.c_str(), &fd_stat) == 0)
      return true;

    fsw_logf_perror(_("Cannot stat %s"), path.c_str());
    return false;
  }

  bool lstat_path(const std::string& path, struct stat& fd_stat)
  {
    if (lstat(path.c_str(), &fd_stat) == 0)
      return true;

    fsw_logf_perror(_("Cannot lstat %s"), path.c_str());
    return false;
  }
}
