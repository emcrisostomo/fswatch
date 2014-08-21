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
#ifndef FSW__CMONITOR_H
#  define FSW__CMONITOR_H

#  include <ctime>

#  ifdef __cplusplus
extern "C"
{
#  endif

  /*
   * This enumeration lists all the available monitors, where the special
   * system_default_monitor_type element refers to the (platform-specific)
   * default monitor.
   */
  enum fsw_monitor_type
  {
    system_default_monitor_type = 0,
    fsevents_monitor_type,
    kqueue_monitor_type,
    inotify_monitor_type,
    poll_monitor_type
  };

#  ifdef __cplusplus
}
#  endif

#endif  /* FSW__CMONITOR_H */

