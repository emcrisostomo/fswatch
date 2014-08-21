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
#ifndef FSW_PATH_UTILS_H
#  define FSW_PATH_UTILS_H

#  include <string>
#  include <vector>
#  include <sys/stat.h>

namespace fsw
{
  void get_directory_children(const std::string &path,
                              std::vector<std::string> &children);
  bool read_link_path(const std::string &path, std::string &link_path);
  bool stat_path(const std::string &path, struct stat &fd_stat);
}
#endif  /* FSW_PATH_UTILS_H */
