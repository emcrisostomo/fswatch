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
#ifndef FSW_EVENT_H
#  define FSW_EVENT_H

#  include <string>
#  include <ctime>
#  include <vector>

enum class event_flag
{
  PlatformSpecific = 1,
  Created = 2,
  Updated = 4,
  Removed = 8,
  Renamed = 16,
  OwnerModified = 32,
  AttributeModified = 64,
  IsFile = 128,
  IsDir = 256,
  IsSymLink = 512,
  Link = 1024
};

class event
{
public:
  event(std::string path, time_t evt_time, std::vector<event_flag> flags);
  virtual ~event();
  std::string get_path() const;
  time_t get_time() const;
  std::vector<event_flag> get_flags() const;

private:
  std::string path;
  time_t evt_time;
  std::vector<event_flag> evt_flags;
};

#endif  /* FSW_EVENT_H */
