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
 * @brief Windows monitor.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef FSW_WINDOWS_MONITOR_H
#  define FSW_WINDOWS_MONITOR_H

#  include "monitor.hpp"
#  include <string>
#  include <vector>

namespace fsw
{
  /**
   * @brief Opaque structure containing implementation specific details of the
   * Windows monitor.
   */
  struct windows_monitor_load;

  /**
   * @brief Windows monitor.
   *
   * This monitor is built upon the `ReadDirectoryChanges` API of the Windows
   * operating systems.
   */
  class windows_monitor : public monitor
  {
    REGISTER_MONITOR(windows_monitor, windows_monitor_type);

  public:
    /**
     * @brief Constructs an instance of this class.
    */
    windows_monitor(std::vector<std::string> paths,
                    FSW_EVENT_CALLBACK *callback,
                    void *context = nullptr);

    /**
     * @brief Destroys an instance of this class.
     */
    virtual ~windows_monitor();

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
    windows_monitor(const windows_monitor& orig) = delete;
    windows_monitor& operator=(const windows_monitor& that) = delete;

    void configure_monitor();
    void initialize_windows_path_list();
    void initialize_events();
    bool init_search_for_path(const std::wstring path);
    void stop_search_for_path(const std::wstring path);
    void process_path(const std::wstring& path);
    bool is_path_watched(std::wstring path);

    // initial load
    windows_monitor_load *load;
  };
}

#endif  /* FSW_WINDOWS_MONITOR_H */
