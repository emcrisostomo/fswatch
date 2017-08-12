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
 * @brief Header of the `libfswatch` library containing common types.
 *
 * This header file defines the types used by the `libfswatch` library.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef LIBFSWATCH_TYPES_H
#define LIBFSWATCH_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Opaque type representing a monitoring session.
 */
struct FSW_SESSION;

/**
 * @brief Handle to a monitoring session.
 */
typedef struct FSW_SESSION *FSW_HANDLE;

/**
 * @brief Status of a library call.
 */
typedef int FSW_STATUS;

#define FSW_INVALID_HANDLE -1

#if defined(HAVE_CXX_THREAD_LOCAL)
# define FSW_THREAD_LOCAL thread_local
#else
# define FSW_THREAD_LOCAL
#endif

#ifdef __cplusplus
}
#endif

#endif /* LIBFSWATCH_TYPES_H */
