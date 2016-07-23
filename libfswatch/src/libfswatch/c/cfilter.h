/*
 * Copyright (c) 2014-2016 Enrico M. Crisostomo
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
 * @brief Header of the `libfswatch` library functions for filter management.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef FSW__CFILTER_H
#  define FSW__CFILTER_H
#  include "cevent.h"

#  ifdef __cplusplus
extern "C"
{
#  endif

  /**
   * @brief Event filter type.
   */
  enum fsw_filter_type
  {
    filter_include,
    filter_exclude
  };

  typedef struct fsw_cmonitor_filter
  {
    char * text;
    enum fsw_filter_type type;
    bool case_sensitive;
    bool extended;
  } fsw_cmonitor_filter;

 /**
  * @brief Event type filter.
  */
  typedef struct fsw_event_type_filter
  {
    enum fsw_event_flag flag;
  } fsw_event_type_filter;

#  ifdef __cplusplus
}
#  endif

#endif  /* FSW__CFILTER_H */
