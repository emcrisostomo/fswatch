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
#ifndef FSW_KQUEUE_MONITOR_H
#  define FSW_KQUEUE_MONITOR_H

#  include "monitor.h"
#  include <string>
#  include <vector>
#  include <sys/stat.h>
#  include <sys/event.h>

namespace fsw
{

  struct kqueue_monitor_load;

  class kqueue_monitor : public monitor
  {
    REGISTER_MONITOR(kqueue_monitor, kqueue_monitor_type);

  public:
    kqueue_monitor(std::vector<std::string> paths,
                   FSW_EVENT_CALLBACK * callback,
                   void * context = nullptr);
    virtual ~kqueue_monitor();

    void run();

  private:
    kqueue_monitor(const kqueue_monitor& orig) = delete;
    kqueue_monitor& operator=(const kqueue_monitor & that) = delete;

    void initialize_kqueue();
    bool scan(const std::string &path);
    bool add_watch(const std::string &path, const struct stat &fd_stat);
    void remove_watch(const std::string &path);
    void remove_watch(int fd);
    bool is_path_watched(const std::string &path);
    void remove_deleted();
    void rescan_pending();
    void scan_root_paths();
    int wait_for_events(const std::vector<struct kevent> &changes,
                        std::vector<struct kevent> &event_list);
    void process_events(const std::vector<struct kevent> &changes,
                        const std::vector<struct kevent> &event_list,
                        int event_num);

    int kq = -1;
    // initial load
    kqueue_monitor_load * load;
  };
}

#endif  /* FSW_KQUEUE_MONITOR_H */
