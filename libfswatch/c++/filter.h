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
#ifndef FSW__FILTER_H
#  define FSW__FILTER_H

#  include <string>
#  include "../c/cfilter.h"

typedef struct monitor_filter
{
  std::string text;
  fsw_filter_type type;
  bool case_sensitive;
  bool extended;
} monitor_filter;

#endif  /* FSW__FILTER_H */
