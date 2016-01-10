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
/**
 * @file
 * @brief Header of the fsw::event class.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */


#ifndef FSW_EVENT_H
#  define FSW_EVENT_H

#  include <string>
#  include <ctime>
#  include <vector>
#  include <iostream>
#  include "../c/cevent.h"

namespace fsw
{
  class event
  {
  public:
    event(std::string path, time_t evt_time, std::vector<fsw_event_flag> flags);
    virtual ~event();

    std::string get_path() const;
    time_t get_time() const;
    std::vector<fsw_event_flag> get_flags() const;

    /**
     * @brief Get event flag by name.
     *
     * @param name The name of the event flag to look for.
     * @return The event flag whose name is @p name, otherwise
     * @exception libfsw_exception if no event flag is found.
     */
    static fsw_event_flag get_event_flag_by_name(const std::string& name);

    /**
     * @brief Get the name of an event flag.
     *
     * @param flag The event flag.
     * @return The name of @p flag.
     * @exception libfsw_exception if no event flag is found.
     */
    static std::string get_event_flag_name(const fsw_event_flag& flag);

  private:
    std::string path;
    time_t evt_time;
    std::vector<fsw_event_flag> evt_flags;
  };

  /**
   * @brief Overload of the `<<` operator to print an event using `iostreams`.
   *
   * @param out A reference to the output stream.
   * @param flag The flag to print.
   * @return A reference to the stream.
   */
  std::ostream& operator<<(std::ostream& out, const fsw_event_flag flag);
}

#endif  /* FSW_EVENT_H */
