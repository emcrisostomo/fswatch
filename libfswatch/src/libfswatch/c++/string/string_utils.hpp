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
/**
 * @file
 * @brief Header of the fsw::string_utils namespace.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef FSW_STRING_UTILS_H
#  define FSW_STRING_UTILS_H

#include <cstdarg>
#include <string>

namespace fsw
{
  /**
   * @brief This namespace contains string manipulation functions.
   */
  namespace string_utils
  {
    /**
     * @brief Create a `std::string` using a `printf()` format and varargs.
     *
     * @param format The `printf()` format.
     * @param ... The arguments to format.
     */
    std::string string_from_format(const char *format, ...);

    /**
     * @brief Create a `std::string` using a `printf()` format and a `va_list`
     * @p args.
     *
     * @param format The `printf()` format.
     * @param args The arguments to format.
     */
    std::string vstring_from_format(const char *format, va_list args);
  }
}

#endif /* FSW_STRING_UTILS_H */
