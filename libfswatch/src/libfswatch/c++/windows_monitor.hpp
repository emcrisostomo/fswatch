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
#ifndef FSW_WINDOWS_MONITOR_H
#  define FSW_WINDOWS_MONITOR_H

#  include "monitor.hpp"
#  include <string>
#  include <vector>
#  include <sys/stat.h>

namespace fsw
{
  struct windows_monitor_load;

  class windows_monitor : public monitor
  {
    REGISTER_MONITOR(windows_monitor, windows_monitor_type);

  public:
    windows_monitor(std::vector<std::string> paths,
                   FSW_EVENT_CALLBACK * callback,
                   void * context = nullptr);
    virtual ~windows_monitor();

    void run();

  private:
    windows_monitor(const windows_monitor& orig) = delete;
    windows_monitor& operator=(const windows_monitor & that) = delete;

    void initialize_windows_path_list();
    void initialize_events();
    void initial_scan();
    bool is_path_watched(const std::wstring path) const;

    // initial load
    windows_monitor_load * load;
  };
}

#endif  /* FSW_WINDOWS_MONITOR_H */
