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
#ifndef FSW__MONITOR_H
#  define FSW__MONITOR_H

#  include "filter.h"
#  include <vector>
#  include <string>
#  include <mutex>
#  include "event.h"
#  include "../c/cmonitor.h"

namespace fsw
{
  typedef void FSW_EVENT_CALLBACK(const std::vector<event> &, void *);

  struct compiled_monitor_filter;

  class monitor
  {
  public:
    monitor(std::vector<std::string> paths,
            FSW_EVENT_CALLBACK * callback,
            void * context = nullptr);
    virtual ~monitor();
    monitor(const monitor& orig) = delete;
    monitor& operator=(const monitor & that) = delete;
    void set_latency(double latency);
    void set_recursive(bool recursive);
    void add_filter(const monitor_filter &filter);
    void set_filters(const std::vector<monitor_filter> &filters);
    void set_follow_symlinks(bool follow);
    void * get_context();
    void set_context(void * context);
    void start();

    static monitor * create_default_monitor(std::vector<std::string> paths,
                                            FSW_EVENT_CALLBACK * callback,
                                            void * context = nullptr);

    static monitor * create_monitor(fsw_monitor_type type,
                                    std::vector<std::string> paths,
                                    FSW_EVENT_CALLBACK * callback,
                                    void * context = nullptr);

  protected:
    bool accept_path(const std::string &path);
    bool accept_path(const char *path);

    virtual void run() = 0;

  protected:
    std::vector<std::string> paths;
    FSW_EVENT_CALLBACK * callback;
    void * context = nullptr;
    double latency = 1.0;
    bool recursive = false;
    bool follow_symlinks = false;

  private:
    std::mutex run_mutex;
    std::vector<compiled_monitor_filter> filters;
  };
}

#endif  /* FSW__MONITOR_H */
