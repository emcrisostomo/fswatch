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
#ifndef FSW__ERROR_H
#  define FSW__ERROR_H

#  ifdef __cplusplus
extern "C"
{
#  endif

  // Error codes
#  define FSW_OK                            0
#  define FSW_ERR_UNKNOWN_ERROR             (1 << 0) 
#  define FSW_ERR_SESSION_UNKNOWN           (1 << 1)
#  define FSW_ERR_MONITOR_ALREADY_EXISTS    (1 << 2)
#  define FSW_ERR_MEMORY                    (1 << 3)
#  define FSW_ERR_UNKNOWN_MONITOR_TYPE      (1 << 4)
#  define FSW_ERR_CALLBACK_NOT_SET          (1 << 5)
#  define FSW_ERR_PATHS_NOT_SET             (1 << 6)
#  define FSW_ERR_UNKNOWN_MONITOR           (1 << 7)
#  define FSW_ERR_MISSING_CONTEXT           (1 << 8)
#  define FSW_ERR_INVALID_PATH              (1 << 9)
#  define FSW_ERR_INVALID_CALLBACK          (1 << 10)
#  define FSW_ERR_INVALID_LATENCY           (1 << 11)
#  define FSW_ERR_INVALID_REGEX             (1 << 12)
#  define FSW_ERR_MONITOR_ALREADY_RUNNING   (1 << 13)
#  define FSW_ERR_STALE_MONITOR_THREAD      (1 << 14)
#  define FSW_ERR_THREAD_FAULT              (1 << 15)
#  define FSW_ERR_UNSUPPORTED_OPERATION     (1 << 16)

#  ifdef __cplusplus
}
#  endif

#endif  /* FSW__ERROR_H */

