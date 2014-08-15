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
#ifndef FSW__CEVENT_H
#  define FSW__CEVENT_H

#  include <ctime>

#  ifdef __cplusplus
extern "C"
{
#  endif

  enum fsw_event_flag
  {
    PlatformSpecific = 1,
    Created = 2,
    Updated = 4,
    Removed = 8,
    Renamed = 16,
    OwnerModified = 32,
    AttributeModified = 64,
    IsFile = 128,
    IsDir = 256,
    IsSymLink = 512,
    Link = 1024
  };

  typedef struct fsw_cevent
  {
    char * path;
    time_t evt_time;
    fsw_event_flag *flags;
    unsigned int flags_num;
  } fsw_cevent;

  typedef void (*FSW_CEVENT_CALLBACK)(fsw_cevent const * const * const events,
    const unsigned int event_num);

#  ifdef __cplusplus
}
#  endif

#endif  /* FSW__CEVENT_H */

