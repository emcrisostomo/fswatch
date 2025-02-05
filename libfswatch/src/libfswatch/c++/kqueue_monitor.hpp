/*
 * Copyright (c) 2014-2021 Enrico M. Crisostomo
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
 * @brief `kqueue` monitor.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef FSW_KQUEUE_MONITOR_H
#  define FSW_KQUEUE_MONITOR_H

#  include "monitor.hpp"
#  include <string>
#  include <vector>
#  include <filesystem>
#  include <sys/stat.h>
#  include <sys/event.h>

namespace fsw
{
  /**
   * @brief Opaque structure containing implementation specific details of the
   * `kqueue` monitor.
   */
  struct kqueue_monitor_load;

  /**
   * @brief Solaris/Illumos monitor.
   *
   * This monitor is built upon the `kqueue` API of the BSD kernels.
   */
  class kqueue_monitor : public monitor
  {
  public:
    /**
     * @brief Constructs an instance of this class.
     */
    kqueue_monitor(std::vector<std::string> paths,
                   FSW_EVENT_CALLBACK *callback,
                   void *context = nullptr);

    /**
     * @brief Destroys an instance of this class.
     */
    ~kqueue_monitor() override;

  protected:
    /**
     * @brief Executes the monitor loop.
     *
     * This call does not return until the monitor is stopped.
     *
     * @see stop()
     */
    void run() final;

  private:
    kqueue_monitor(const kqueue_monitor& orig) = delete;
    kqueue_monitor& operator=(const kqueue_monitor& that) = delete;

    void initialize_kqueue();
    void terminate_kqueue();
    void scan(const std::filesystem::path& path);
    bool add_watch(const std::string& path, const struct stat& fd_stat);
    bool is_path_watched(const std::string& path) const;
    void remove_deleted();
    void rescan_pending();
    void scan_root_paths();
    int wait_for_events(const std::vector<struct kevent>& changes,
                        std::vector<struct kevent>& event_list) const;
    void process_events(const std::vector<struct kevent>& event_list,
                        int event_num);

    int kq = -1;
    // initial load
    std::unique_ptr<kqueue_monitor_load> load;
  };
}

#endif  /* FSW_KQUEUE_MONITOR_H */
