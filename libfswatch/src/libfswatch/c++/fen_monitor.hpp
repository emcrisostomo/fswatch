/*
 * Copyright (c) 2015-2016 Enrico M. Crisostomo
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

#ifndef FSW_FEN_MONITOR_H
#  define FSW_FEN_MONITOR_H

#  include "monitor.hpp"
#  include <string>
#  include <vector>

namespace fsw
{
  /**
   * @brief Opaque structure containing implementation specific details of the
   * Solaris/Illumos monitor.
   */
  struct fen_monitor_load;

  /**
   * @brief Opaque structure containing implementation specific details of the
   * Solaris/Illumos monitor.
   */
  struct fen_info;

  /**
   * @brief Solaris/Illumos monitor.
   *
   * This monitor is built upon the _File Events Notification_ API of the
   * Solaris and Illumos kernels.
   */
  class fen_monitor : public monitor
  {
    REGISTER_MONITOR(fen_monitor, fen_monitor_type);

  public:
    /**
     * @brief Constructs an instance of this class.
     */
    fen_monitor(std::vector<std::string> paths,
                FSW_EVENT_CALLBACK *callback,
                void *context = nullptr);

    /**
     * @brief Destroys an instance of this class.
     */
    virtual ~fen_monitor();

  protected:
    /**
     * @brief Executes the monitor loop.
     *
     * This call does not return until the monitor is stopped.
     *
     * @see stop()
     */
    void run() override;

  private:
    fen_monitor(const fen_monitor& orig) = delete;
    fen_monitor& operator=(const fen_monitor& that) = delete;

    void scan_root_paths();
    bool scan(const std::string& path, bool is_root_path = true);
    bool is_path_watched(const std::string& path) const;
    bool add_watch(const std::string& path, const struct stat& fd_stat);
    bool associate_port(struct fen_info *finfo, const struct stat& fd_stat);
    void process_events(struct fen_info *obj, int events);
    void rescan_removed();
    void rescan_pending();

    // pimpl
    fen_monitor_load *load;
  };
}

#endif  /* FSW_FEN_MONITOR_H */
