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
#ifndef FSW_INOTIFY_MONITOR_H
#  define FSW_INOTIFY_MONITOR_H

#  include "monitor.h"
#  include <sys/inotify.h>
#  include <string>
#  include <vector>
#  include <sys/stat.h>

namespace fsw
{
  struct inotify_monitor_impl;

  class inotify_monitor : public monitor
  {
    REGISTER_MONITOR(inotify_monitor, inotify_monitor_type);

  public:
    inotify_monitor(std::vector<std::string> paths,
                    FSW_EVENT_CALLBACK * callback,
                    void * context = nullptr);
    virtual ~inotify_monitor();

    void run();

  private:
    inotify_monitor(const inotify_monitor& orig) = delete;
    inotify_monitor& operator=(const inotify_monitor & that) = delete;

    void scan_root_paths();
    bool is_watched(const std::string & path);
    void notify_events();
    void preprocess_dir_event(struct inotify_event * event);
    void preprocess_event(struct inotify_event * event);
    void preprocess_node_event(struct inotify_event * event);
    void scan(const std::string &path, const bool accept_non_dirs = true);
    bool add_watch(const std::string &path,
                   const struct stat &fd_stat);
    void process_pending_events();
    void remove_watch(int fd);

    inotify_monitor_impl * impl;
  };
}

#endif  /* FSW_INOTIFY_MONITOR_H */
