/*
 * Copyright (c) 2015 Enrico M. Crisostomo
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
#ifndef FSW_FEN_MONITOR_H
#  define FSW_FEN_MONITOR_H

#  include "monitor.hpp"
#  include <string>
#  include <vector>

namespace fsw
{
  struct fen_monitor_load;
  struct fen_info;

  class fen_monitor : public monitor
  {
    REGISTER_MONITOR(fen_monitor, fen_monitor_type);

  public:
    fen_monitor(std::vector<std::string> paths,
                FSW_EVENT_CALLBACK *callback,
                void *context = nullptr);
    virtual ~fen_monitor();

    void run();

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
