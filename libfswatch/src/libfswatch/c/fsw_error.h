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
 * @brief Error values.
 *
 * This header file defines the error values used by the `libfswatch` API.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */
#ifndef FSW__ERROR_H
#  define FSW__ERROR_H

#  ifdef __cplusplus
extern "C"
{
#  endif

// Error codes
#  define FSW_OK                            0         /**< The call was successful. */
#  define FSW_ERR_UNKNOWN_ERROR             (1 << 0)  /**< An unknown error has occurred. */
#  define FSW_ERR_SESSION_UNKNOWN           (1 << 1)  /**< The session specified by the handle is unknown. */
#  define FSW_ERR_MONITOR_ALREADY_EXISTS    (1 << 2)  /**< The session already contains a monitor. */
#  define FSW_ERR_MEMORY                    (1 << 3)  /**< An error occurred while invoking a memory management routine. */
#  define FSW_ERR_UNKNOWN_MONITOR_TYPE      (1 << 4)  /**< The specified monitor type does not exist. */
#  define FSW_ERR_CALLBACK_NOT_SET          (1 << 5)  /**< The callback has not been set. */
#  define FSW_ERR_PATHS_NOT_SET             (1 << 6)  /**< The paths to watch have not been set. */
#  define FSW_ERR_MISSING_CONTEXT           (1 << 7)  /**< The callback context has not been set. */
#  define FSW_ERR_INVALID_PATH              (1 << 8)  /**< The path is invalid. */
#  define FSW_ERR_INVALID_CALLBACK          (1 << 9)  /**< The callback is invalid. */
#  define FSW_ERR_INVALID_LATENCY           (1 << 10) /**< The latency is invalid. */
#  define FSW_ERR_INVALID_REGEX             (1 << 11) /**< The regular expression is invalid. */
#  define FSW_ERR_MONITOR_ALREADY_RUNNING   (1 << 12) /**< A monitor is already running in the specified session. */
#  define FSW_ERR_UNKNOWN_VALUE             (1 << 13) /**< The value is unknown. */
#  define FSW_ERR_INVALID_PROPERTY          (1 << 14) /**< The property is invalid. */

#  ifdef __cplusplus
}
#  endif

#endif  /* FSW__ERROR_H */
