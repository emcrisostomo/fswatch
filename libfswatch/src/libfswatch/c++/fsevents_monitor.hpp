/*
 * Copyright (c) 2014-2018 Enrico M. Crisostomo
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
  public:

    /**
     * @brief Custom monitor property used to enable the kFSEventStreamCreateFlagNoDefer flag in the event stream.
     *
     * If you specify this flag and more than latency seconds have elapsed since
     * the last event, your app will receive the event immediately.  The
     * delivery of the event resets the latency timer and any further events
     * will be delivered after latency seconds have elapsed.  This flag is
     * useful for apps that are interactive and want to react immediately to
     * changes but avoid getting swamped by notifications when changes are
     * occurring in rapid succession.  If you do not specify this flag, then
     * when an event occurs after a period of no events, the latency timer is
     * started. Any events that occur during the next latency seconds will be
     * delivered as one group (including that first event).  The delivery of the
     * group of events resets the latency timer and any further events will be
     * delivered after latency seconds.  This is the default behavior and is
     * more appropriate for background, daemon or batch processing apps.
     *
     * @sa https://developer.apple.com/documentation/coreservices/kfseventstreamcreateflagnodefer
     */
    static constexpr const char *DARWIN_EVENTSTREAM_NO_DEFER = "darwin.eventStream.noDefer";

    /**
     * @brief Constructs an instance of this class.
     */
    fsevents_monitor(std::vector<std::string> paths,
                     FSW_EVENT_CALLBACK *callback,
                     void *context = nullptr);
    fsevents_monitor(const fsevents_monitor& orig) = delete;
    fsevents_monitor& operator=(const fsevents_monitor& that) = delete;

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
    static void fsevents_callback(ConstFSEventStreamRef streamRef,
                                  void *clientCallBackInfo,
                                  size_t numEvents,
                                  void *eventPaths,
                                  const FSEventStreamEventFlags eventFlags[],
                                  const FSEventStreamEventId eventIds[]);

    FSEventStreamRef stream = nullptr;
    CFRunLoopRef run_loop = nullptr;
    bool no_defer();
  };
}

#endif  /* FSW_FSEVENT_MONITOR_H */
