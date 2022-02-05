/*
 * Copyright (c) 2014-2015 Enrico M. Crisostomo
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
/**
 * @file
 * @brief Header defining utility functions to manipulate paths.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef FSW_PATH_UTILS_H
#  define FSW_PATH_UTILS_H

#  include <string>
#  include <vector>
#  include <sys/stat.h>
#  include "cxxfswatch_export.h"
#  include "libfswatch/libfswatch_config.h"

#ifdef _MSC_VER
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

namespace fsw
{
  /**
   * @brief A thin wrapper about realpath.
   *
   * @param path The path to resolve.
   * @param resolved_path A pointer to a buffer where the resolved path is
   * stored.
   * @return If there is no error, realpath() returns a string, otherwise it
   * throws a std::system_error.
   */
  CXXFSWATCH_EXPORT std::string fsw_realpath(const char *path, char *resolved_path);

  /**
   * @brief Gets a vector of direct directory children.
   *
   * @param path The directory whose children must be returned.
   * @return A vector containing the list of children of @p path.
   */
  CXXFSWATCH_EXPORT std::vector<std::string> get_directory_children(const std::string& path);

  /**
   * @brief Resolves a path name.
   *
   * This function resolves @p path using @c realpath() and stores the absolute
   * pathname into @p link_path.  The function returns @c true if it succeeds,
   * @c false otherwise.
   *
   * @param path The path to resolve.
   * @param link_path A reference to a `std::string` where the resolved absolute
   * path should be copied to.
   * @return @c true if the function succeeds, @c false otherwise.
   */
  CXXFSWATCH_EXPORT bool read_link_path(const std::string& path, std::string& link_path);

  /**
   * @brief Wraps a @c lstat(path, fd_stat) call that invokes @c perror() if it
   * fails.
   *
   * @param path The path to @c lstat().
   * @param fd_stat The @c stat structure where @c lstat() writes its results.
   * @return @c true if the function succeeds, @c false otherwise.
   */
  CXXFSWATCH_EXPORT bool lstat_path(const std::string& path, struct stat& fd_stat);

  /**
   * @brief Wraps a @c stat(path, fd_stat) call that invokes @c perror() if it
   * fails.
   *
   * @param path The path to @c stat().
   * @param fd_stat The @c stat structure where @c stat() writes its results.
   * @return @c true if the function succeeds, @c false otherwise.
   */
  CXXFSWATCH_EXPORT bool stat_path(const std::string& path, struct stat& fd_stat);
}
#endif  /* FSW_PATH_UTILS_H */
