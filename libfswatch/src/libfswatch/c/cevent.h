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
 * @brief Event type manipulation.
 *
 * This header file defines the event types of the `libfswatch` API.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */
#ifndef FSW__CEVENT_H
#  define FSW__CEVENT_H

#  include <time.h>
#  include <limits.h>
#  include "libfswatch_types.h"

#  ifdef __cplusplus
extern "C"
{
#  endif

/**
 * @brief Backend-agnostic change flags.
 *
 * Each element of this enum represents a backend-agnostic change flag.  No
 * direct mapping to backend-specific change types is guaranteed to exist: a
 * change type may be mapped to multiple `fsw_event_flag` instances included
 * the `PlatformSpecific` flag.
 *
 * The values of event flags are all powers of 2, that is numbers @f$f=2^n@f$
 * where @f$n@f$ is an integer.  This representation makes it easy to combine
 * flags into a bit mask and encode multiple events flags into a single integer.
 *
 * A monitor implementation is required to map implementation-specific flags
 * into API flags.  Sometimes, though, a perfect match is not possible and the
 * following situation may arise:
 *
 *   - One platform-specific flag must be mapped into multiple API flags.
 *
 *   - Multiple platform-specific flags must be mapped into a single API flag.
 *
 *   - A mapping is not possible for some flags, in which case they should be
 *     mapped to fsw_event_flag::PlatformSpecific.  The API currently offers no
 *     way to retain a platform-specific event flag value in this case.
 */
  enum fsw_event_flag
  {
    NoOp = 0,                     /**< No event has occurred. */
    PlatformSpecific = (1 << 0),  /**< Platform-specific placeholder for event type that cannot currently be mapped. */
    Created = (1 << 1),           /**< An object was created. */
    Updated = (1 << 2),           /**< An object was updated. */
    Removed = (1 << 3),           /**< An object was removed. */
    Renamed = (1 << 4),           /**< An object was renamed. */
    OwnerModified = (1 << 5),     /**< The owner of an object was modified. */
    AttributeModified = (1 << 6), /**< The attributes of an object were modified. */
    MovedFrom = (1 << 7),         /**< An object was moved from this location. */
    MovedTo = (1 << 8),           /**< An object was moved to this location. */
    IsFile = (1 << 9),            /**< The object is a file. */
    IsDir = (1 << 10),            /**< The object is a directory. */
    IsSymLink = (1 << 11),        /**< The object is a symbolic link. */
    Link = (1 << 12),             /**< The link count of an object has changed. */
    Overflow = (1 << 13)          /**< The event queue has overflowed. */
  };

  extern fsw_event_flag FSW_ALL_EVENT_FLAGS[15];

  /**
   * @brief Get event flag by name.
   *
   * This function looks for an event flag called @p name and, if it exists, it
   * writes its value onto @p flag and @c FSW_OK, otherwise @p flag is not
   * modified and @c FSW_ERR_UNKNOWN_VALUE is returned.
   *
   * @param[in] name The name of the event flag to look for.
   * @param[out] flag The output variable where the event flag is returned.
   * @return #FSW_OK if the functions succeeds, #FSW_ERR_UNKNOWN_VALUE
   * otherwise.
   */
  FSW_STATUS fsw_get_event_flag_by_name(const char *name, enum fsw_event_flag *flag);

  /**
   * @brief Get the name of an event flag.
   *
   * This function looks for the name of the specified event @p flag.  If it
   * exists, it returns its name, otherwise @c nullptr is returned.
   *
   * @param[in] flag The event flag to look for.
   * @return The name of @p flag, or @c nullptr if it does not exist.
   */
  char *fsw_get_event_flag_name(const enum fsw_event_flag flag);

  /**
   * A file change event is represented as an instance of this struct where:
   *   - path is the path where the event was triggered.
   *   - evt_time the time when the event was triggered.
   *   - flags is an array of fsw_event_flag of size flags_num.
   *   - flags_num is the size of the flags array.
   */
  typedef struct fsw_cevent
  {
    char * path;
    time_t evt_time;
    enum fsw_event_flag * flags;
    unsigned int flags_num;
  } fsw_cevent;

  /**
   * A function pointer of type FSW_CEVENT_CALLBACK is used by the API as a
   * callback to provide information about received events.  The callback is
   * passed the following arguments:
   *   - events, a const pointer to an array of events of type const fsw_cevent.
   *   - event_num, the size of the *events array.
   *   - data, optional persisted data for a callback.
   *
   * The memory used by the fsw_cevent objects will be freed at the end of the
   * callback invocation.  A callback should copy such data instead of storing
   * a pointer to it.
   */
  typedef void (*FSW_CEVENT_CALLBACK)(fsw_cevent const *const events,
                                      const unsigned int event_num,
                                      void *data);

#  ifdef __cplusplus
}
#  endif

#endif  /* FSW__CEVENT_H */
