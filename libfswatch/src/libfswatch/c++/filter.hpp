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
 * @brief Header of the fsw::monitor_filter class.
 *
 * This header file defines the fsw::monitor_filter class, a type that
 * represents a path filter.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */
#ifndef FSW__FILTER_H
#  define FSW__FILTER_H

#  include <string>
#  include "../c/cfilter.h"
#  include <vector>

namespace fsw
{
  /**
   * @brief Path filters used to accept or reject file change events.
   *
   * A path filter is a regular expression used to accept or reject file change
   * events based on the value of their path.  A filter has the following
   * characteristics:
   *
   *   - It has a _regular expression_ (monitor_filter::text), used to match the
   *     paths.
   *
   *   - It can be an _inclusion_ or an _exclusion_ filter
   *     (monitor_filter::type).
   *
   *   - It can be case _sensitive_ or _insensitive_
   *     (monitor_filter::case_sensitive).
   *
   *   - It can be an _extended_ regular expression (monitor_filter::extended).
   *
   * Further information about how filtering works in `libfswatch` can be found
   * in @ref path-filtering.
   */
  typedef struct monitor_filter
  {
    /**
     * @brief Regular expression used to match the paths.
     *
     * Further information about regular expressions can be found here:
     *
     * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap09.html
     */
    std::string text;

    /**
     * @brief Filter type.
     */
    fsw_filter_type type;

    /**
     * @brief Flag indicating whether monitor_filter::text is a case sensitive
     * regular expression.
     */
    bool case_sensitive;

    /**
     * @brief Flag indicating whether monitor_filter::text is an extended
     * regular expression.
     *
     * Further information about extended regular expressions can be found here:
     *
     * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap09.html#tag_09_04
     */
    bool extended;

    /**
     * @brief Load filters from the specified file.
     *
     * Filters can be loaded from a text file containing one filter per line.
     * A filter has the following structure:
     *
     *   - It is validated by the following regular expression:
     *     `^([+-])([ei]*) (.+)$`
     *
     *   - The first character is the filter type: `+` if it is an _inclusion_
     *     filter, `-` if it is an _exclusion_ filter.
     *
     *   - An optional list of flags:
     *
     *     - `e` if it is an _extended_ regular expression.
     *
     *     - `i` if it is a _case insensitive_ regular expression.
     *
     *   - A space.
     *
     *   - The filter regular expression text.
     *
     * Parsing errors are notified through an optional error handler.  The valid
     * filters are returned in a vector.
     *
     * @param path The path of the file to read filters from.
     * @param err_handler An optional error handler.
     * @return A vector containing the valid filters.
     * @throw invalid_argument If the specified path cannot be opened.
     */
    static std::vector<monitor_filter> read_from_file(const std::string& path,
                                                      void (*err_handler)(
                                                        std::string) = nullptr);
  } monitor_filter;
}

#endif  /* FSW__FILTER_H */
