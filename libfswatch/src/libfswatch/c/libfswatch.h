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
 * @brief Header of the `libfswatch` library.
 *
 * This header file defines the API of the `libfswatch` library.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef LIBFSW_H
#define LIBFSW_H

#include <stdbool.h>
#include "libfswatch_types.h"
#include "cevent.h"
#include "cmonitor.h"
#include "cfilter.h"
#include "error.h"

#  ifdef __cplusplus
extern "C"
{
#  endif

  /**
   * The `libfswatch` C API let users create monitor sessions and receive file
   * system events matching the specified criteria.  Most API functions return
   * a status code of type FSW_STATUS which can take any value specified in
   * the error.h header.  A successful API call returns FSW_OK and the last
   * error can be obtained calling the fsw_last_error() function.
   *
   * If the compiler and the C++ library used to build `libfswatch` support the
   * thread_local storage specified then this API is thread safe and a
   * different state is maintained on a per-thread basis.
   *
   * Session-modifying API calls (such as fsw_add_path) will take effect the
   * next time a monitor is started with fsw_start_monitor.
   *
   * Currently not all monitors supports being stopped, in which case
   * fsw_start_monitor is a non-returning API call.
   *
   * A basic session needs at least:
   *
   *   * A path to watch.
   *   * A callback to process the events sent by the monitor.
   *
   * as shown in the next example (error checking code was omitted).
   *
   *     // Use the default monitor.
   *     const FSW_HANDLE handle = fsw_init_session(system_default_monitor_type);
   *
   *     fsw_add_path(handle, "my/path");
   *     fsw_set_callback(handle, my_callback);
   *
   *     fsw_start_monitor(handle);
   *
   * A suitable callback function is a function pointer of type
   * FSW_CEVENT_CALLBACK, that is it is a function conforming with the
   * following signature:
   *
   *     void c_process_events(fsw_cevent const * const events,
   *                           const unsigned int event_num,
   *                           void * data);
   *
   * When a monitor receives change events satisfying all the session criteria,
   * the callback is invoked and passed a copy of the events.
   */

  /**
   * This function initializes the `libfswatch` library and must be invoked
   * before any other calls to the C or C++ API.  If the function succeeds, it
   * returns FSW_OK, otherwise the initialization routine failed and the library
   * should not be usable.
   */
  FSW_STATUS fsw_init_library();

  /**
   * This function creates a new monitor session using the specified monitor
   * and returns an handle to it.  This function is the `libfswatch` API entry
   * point.
   *
   * @see cmonitor.h for a list of all the available monitors.
   */
  FSW_HANDLE fsw_init_session(const enum fsw_monitor_type type);

  /**
   * Adds a path to watch to the specified session.  At least one path must be
   * added to the current session in order for it to be valid.
   */
  FSW_STATUS fsw_add_path(const FSW_HANDLE handle, const char * path);

  /**
   * Adds the specified monitor property.
   */
  FSW_STATUS fsw_add_property(const FSW_HANDLE handle, const char * name, const char * value);

  /**
   * Sets the allow overflow flag of the monitor.  When this flag is set, a
   * monitor is allowed to overflow and report it as a change event.
   */
  FSW_STATUS fsw_set_allow_overflow(const FSW_HANDLE handle, const bool allow_overflow);

  /**
   * Sets the callback the monitor invokes when some events are received.  The
   * callback must be set in the current session in order for it to be valid.
   *
   * See cevent.h for the definition of FSW_CEVENT_CALLBACK.
   */
  FSW_STATUS fsw_set_callback(const FSW_HANDLE handle,
                              const FSW_CEVENT_CALLBACK callback,
                              void * data);

  /**
   * Sets the latency of the monitor.  By default, the latency is set to 1 s.
   */
  FSW_STATUS fsw_set_latency(const FSW_HANDLE handle, const double latency);

  /**
   * Determines whether the monitor recursively scans each watched path or not.
   * Recursive scanning is an optional feature which could not be implemented
   * by all the monitors.  By default, recursive scanning is disabled.
   */
  FSW_STATUS fsw_set_recursive(const FSW_HANDLE handle, const bool recursive);

  /**
   * Determines whether the monitor only watches a directory when performing a
   * recursive scan.  By default, a monitor accepts all kinds of files.
   */
  FSW_STATUS fsw_set_directory_only(const FSW_HANDLE handle, const bool directory_only);

  /**
   * Determines whether a symbolic link is followed or not.  By default, a
   * symbolic link are not followed.
   */
  FSW_STATUS fsw_set_follow_symlinks(const FSW_HANDLE handle,
                                     const bool follow_symlinks);

  /**
   * Adds an event type filter to the current session.
   *
   * See cfilter.h for the definition of fsw_event_type_filter.
   */
  FSW_STATUS fsw_add_event_type_filter(const FSW_HANDLE handle,
                                       const fsw_event_type_filter event_type);

  /**
   * Adds a filter to the current session.  A filter is a regular expression
   * that, depending on whether the filter type is exclusion or not, must or
   * must not be matched for an event path for the event to be accepted.
   *
   * See cfilter.h for the definition of fsw_cmonitor_filter.
   */
  FSW_STATUS fsw_add_filter(const FSW_HANDLE handle,
                            const fsw_cmonitor_filter filter);

  /**
   * Starts the monitor if it is properly configured.  Depending on the type of
   * monitor this call might return when a monitor is stopped or not.
   */
  FSW_STATUS fsw_start_monitor(const FSW_HANDLE handle);

  /**
   * Stops a running monitor.
   */
  FSW_STATUS fsw_stop_monitor(const FSW_HANDLE handle);

  /**
   * Destroys an existing session and invalidates its handle.
   */
  FSW_STATUS fsw_destroy_session(const FSW_HANDLE handle);

  /**
   * Gets the last error code.
   */
  FSW_STATUS fsw_last_error();

  /**
   * Check whether the verbose mode is active.
   */
  bool fsw_is_verbose();

  /**
   * Set the verbose mode.
   */
  void fsw_set_verbose(bool verbose);

#  ifdef __cplusplus
}
#  endif

#endif /* LIBFSW_H */
