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
 * @brief Header of the `libfswatch` library defining the monitor types.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef FSW__CMONITOR_H
#  define FSW__CMONITOR_H

#  include <time.h>

#  ifdef __cplusplus
extern "C"
{
#  endif

  /**
   * @brief Available monitors.
   *
   * This enumeration lists all the available monitors, where the special
   * ::system_default_monitor_type element refers to the platform-specific
   * default monitor.
   */
  enum fsw_monitor_type
  {
    system_default_monitor_type = 0, /**< System default monitor. */
    fsevents_monitor_type,           /**< OS X FSEvents monitor. */
    kqueue_monitor_type,             /**< BSD `kqueue` monitor. */
    inotify_monitor_type,            /**< Linux `inotify` monitor. */
    windows_monitor_type,            /**< Windows monitor. */
    poll_monitor_type,               /**< `stat()`-based poll monitor. */
    fen_monitor_type                 /**< Solaris/Illumos monitor. */
  };

#  ifdef __cplusplus
}
#  endif

#endif  /* FSW__CMONITOR_H */
