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
#ifndef FSW__CFILTER_H
#  define FSW__CFILTER_H

#  ifdef __cplusplus
extern "C"
{
#  endif

  enum fsw_filter_type
  {
    filter_include,
    filter_exclude
  };

  typedef struct fsw_cmonitor_filter
  {
    char * text;
    fsw_filter_type type;
    bool case_sensitive;
    bool extended;
  } fsw_cmonitor_filter;

#  ifdef __cplusplus
}
#  endif

#endif  /* FSW__CFILTER_H */
