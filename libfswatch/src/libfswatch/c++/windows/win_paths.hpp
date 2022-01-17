/*
 * Copyright (c) 2016 Enrico M. Crisostomo
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
 * @brief Header of the fsw::win_paths namespace.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef FSW_WIN_PATHS_HPP
#  define  FSW_WIN_PATHS_HPP

#  include <string>
#  include "fswatch_cxx_windows_export.h"

namespace fsw
{
  /**
   * @brief Path conversion functions.
   *
   * This namespace contains utility functions for POSIX to Windows and Windows
   * to POSIX path conversion functions.
   */
  namespace win_paths
  {
    /**
     * @brief Converts a POSIX path to Windows.
     *
     * @param path The POSIX path to convert to a Windows path.
     * @return The converted Windows path.
     */
    FSWATCH_CXX_WINDOWS_EXPORT std::wstring posix_to_win_w(const std::string &path);

    /**
     * @brief Converts a Windows path to POSIX.
     *
     * @param path The Windows path to convert to POSIX.
     * @return The converted POSIX path.
     */
    FSWATCH_CXX_WINDOWS_EXPORT std::string win_w_to_posix(const std::wstring &path);
  }
}
#endif	/* FSW_WIN_PATHS_HPP */

