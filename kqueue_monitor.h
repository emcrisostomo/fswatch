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

#  include "config.h"

#  ifdef HAVE_SYS_EVENT_H

#    include "monitor.h"
#    include "fswatch_map.h"
#    include "fswatch_set.h"
#    include <string>
#    include <vector>
#    include <sys/stat.h>

class kqueue_monitor : public monitor
{
public:
  kqueue_monitor(std::vector<std::string> paths, EVENT_CALLBACK callback);
  virtual ~kqueue_monitor();

  void run();

private:
  kqueue_monitor(const kqueue_monitor& orig);
  kqueue_monitor& operator=(const kqueue_monitor & that);

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
  fsw_hash_map<std::string, int> descriptors_by_file_name;
  fsw_hash_map<int, std::string> file_names_by_descriptor;
  fsw_hash_set<int> descriptors_to_remove;
  fsw_hash_set<int> descriptors_to_rescan;
  fsw_hash_map<int, mode_t> file_modes;

  static const unsigned int MIN_SPIN_LATENCY = 1;
};

#  endif  /* HAVE_SYS_EVENT_H */
#endif  /* FSW_KQUEUE_MONITOR_H */
