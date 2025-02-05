/*
 * Copyright (c) 2014-2024 Enrico M. Crisostomo
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
 * @brief `stat()` based monitor.
 *
 * @copyright Copyright (c) 2014-2024 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef FSW_POLL_MONITOR_H
#  define FSW_POLL_MONITOR_H

#  include "monitor.hpp"
#  include <sys/stat.h>
#  include <ctime>
#  include <memory>
#  include <filesystem>
#  include <functional>

namespace fsw
{
  /**
   * @brief `stat()`-based monitor.
   *
   * This monitor uses the `stat()` function to periodically check the observed
   * paths and detect changes.
   */
  class poll_monitor : public monitor
  {
  public:
    /**
     * @brief Constructs an instance of this class.
     */
    poll_monitor(std::vector<std::string> paths,
                 FSW_EVENT_CALLBACK *callback,
                 void *context = nullptr);

    /**
     * @brief Destroys an instance of this class.
     */
    ~poll_monitor() override;

  protected:
    void run() override;

  private:
    static const unsigned int MIN_POLL_LATENCY = 1;

    poll_monitor(const poll_monitor& orig) = delete;
    poll_monitor& operator=(const poll_monitor& that) = delete;

    using path_visitor = std::function<bool(const std::string&, const struct stat&)>;

    struct watched_file_info
    {
      time_t mtime;
      time_t ctime;
    };

    using watched_file_info = struct watched_file_info;

    struct poll_monitor_data;

    void scan(const std::filesystem::path& path, const path_visitor& fn);
    void collect_initial_data();
    void collect_data();
    bool add_path(const std::string& path,
                  const struct stat& fd_stat,
                  const path_visitor& poll_callback);
    bool initial_scan_callback(const std::string& path, const struct stat& stat);
    bool intermediate_scan_callback(const std::string& path,
                                    const struct stat& stat);
    void find_removed_files();
    void swap_data_containers();

    std::unique_ptr<poll_monitor_data> previous_data;
    std::unique_ptr<poll_monitor_data> new_data;

    std::vector<event> events;
    time_t curr_time;
  };
}

#endif  /* FSW_POLL_MONITOR_H */
