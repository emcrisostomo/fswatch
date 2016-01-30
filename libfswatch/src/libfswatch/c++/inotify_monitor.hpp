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
 * @brief Solaris/Illumos monitor.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef FSW_INOTIFY_MONITOR_H
#  define FSW_INOTIFY_MONITOR_H

#  include "monitor.hpp"
#  include <sys/inotify.h>
#  include <string>
#  include <vector>
#  include <sys/stat.h>

namespace fsw
{
  /**
   * @brief Opaque structure containing implementation specific details of the
   * FSEvents monitor.
   */
  struct inotify_monitor_impl;

  /**
   * @brief Solaris/Illumos monitor.
   *
   * This monitor is built upon the _File Events Notification_ API of the
   * Solaris and Illumos kernels.
   */
  class inotify_monitor : public monitor
  {
    REGISTER_MONITOR(inotify_monitor, inotify_monitor_type);

  public:
    /**
     * @brief Constructs an instance of this class.
     */
    inotify_monitor(std::vector<std::string> paths,
                    FSW_EVENT_CALLBACK *callback,
                    void *context = nullptr);

    /**
     * @brief Destroys an instance of this class.
     */
    virtual ~inotify_monitor();

  protected:
    /**
     * @brief Executes the monitor loop.
     *
     * This call does not return until the monitor is stopped.
     *
     * @see stop()
     */
    void run();

  private:
    inotify_monitor(const inotify_monitor& orig) = delete;
    inotify_monitor& operator=(const inotify_monitor& that) = delete;

    void scan_root_paths();
    bool is_watched(const std::string& path) const;
    void preprocess_dir_event(struct inotify_event *event);
    void preprocess_event(struct inotify_event *event);
    void preprocess_node_event(struct inotify_event *event);
    void scan(const std::string& path, const bool accept_non_dirs = true);
    bool add_watch(const std::string& path,
                   const struct stat& fd_stat);
    void process_pending_events();
    void remove_watch(int fd);

    inotify_monitor_impl *impl;
  };
}

#endif  /* FSW_INOTIFY_MONITOR_H */
