/*
 * Copyright (c) 2014-2016 Enrico M. Crisostomo
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
 * @brief OS X FSEvents monitor.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef FSW_FSEVENT_MONITOR_H
#  define FSW_FSEVENT_MONITOR_H

#  include "monitor.hpp"
#  include <CoreServices/CoreServices.h>

namespace fsw
{
  /**
   * @brief OS X FSEvents monitor.
   *
   * This monitor is built upon the _FSEvents_ API of the Apple OS X kernel.
   */
  class fsevents_monitor : public monitor
  {
    REGISTER_MONITOR(fsevents_monitor, fsevents_monitor_type);

  public:
    /**
     * @brief Constructs an instance of this class.
     */
    fsevents_monitor(std::vector<std::string> paths,
                     FSW_EVENT_CALLBACK *callback,
                     void *context = nullptr);
    /**
     * @brief Destroys an instance of this class.
     */
    virtual ~fsevents_monitor();

  protected:
    /**
     * @brief Executes the monitor loop.
     *
     * This call does not return until the monitor is stopped.
     *
     * @see stop()
     */
    void run() override;

    /**
     * @brief Execute an implementation-specific stop handler.
     */
    void on_stop() override;

  private:
    fsevents_monitor(const fsevents_monitor& orig) = delete;
    fsevents_monitor& operator=(const fsevents_monitor& that) = delete;

    static void fsevents_callback(ConstFSEventStreamRef streamRef,
                                  void *clientCallBackInfo,
                                  size_t numEvents,
                                  void *eventPaths,
                                  const FSEventStreamEventFlags eventFlags[],
                                  const FSEventStreamEventId eventIds[]);

    FSEventStreamRef stream = nullptr;
    CFRunLoopRef run_loop = nullptr;
  };
}

#endif  /* FSW_FSEVENT_MONITOR_H */
