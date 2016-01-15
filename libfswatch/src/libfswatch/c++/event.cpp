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
#include "gettext_defs.h"
#include "event.hpp"
#include "libfswatch_exception.hpp"
#include <map>

using namespace std;

namespace fsw
{
  event::event(string path, time_t evt_time, vector<fsw_event_flag> flags) :
    path(std::move(path)), evt_time(evt_time), evt_flags(std::move(flags))
  {
  }

  event::~event()
  {
  }

  string event::get_path() const
  {
    return path;
  }

  time_t event::get_time() const
  {
    return evt_time;
  }

  vector<fsw_event_flag> event::get_flags() const
  {
    return evt_flags;
  }

  fsw_event_flag event::get_event_flag_by_name(const string& name)
  {
#define FSW_MAKE_PAIR_FROM_NAME(p) {#p, p}
    static const map<string, fsw_event_flag> flag_by_names = {
      FSW_MAKE_PAIR_FROM_NAME(NoOp),
      FSW_MAKE_PAIR_FROM_NAME(PlatformSpecific),
      FSW_MAKE_PAIR_FROM_NAME(Created),
      FSW_MAKE_PAIR_FROM_NAME(Updated),
      FSW_MAKE_PAIR_FROM_NAME(Removed),
      FSW_MAKE_PAIR_FROM_NAME(Renamed),
      FSW_MAKE_PAIR_FROM_NAME(OwnerModified),
      FSW_MAKE_PAIR_FROM_NAME(AttributeModified),
      FSW_MAKE_PAIR_FROM_NAME(MovedFrom),
      FSW_MAKE_PAIR_FROM_NAME(MovedTo),
      FSW_MAKE_PAIR_FROM_NAME(IsFile),
      FSW_MAKE_PAIR_FROM_NAME(IsDir),
      FSW_MAKE_PAIR_FROM_NAME(IsSymLink),
      FSW_MAKE_PAIR_FROM_NAME(Link),
      FSW_MAKE_PAIR_FROM_NAME(Overflow)
    };
#undef FSW_MAKE_PAIR_FROM_NAME

    auto flag = flag_by_names.find(name);

    if (flag == flag_by_names.end())
      throw libfsw_exception(_("Unknown event type: ") + name, FSW_ERR_UNKNOWN_VALUE);

    return flag->second;
  }

  string event::get_event_flag_name(const fsw_event_flag& flag)
  {
#define FSW_MAKE_PAIR_FROM_NAME(p) {p, #p}
    static const map<fsw_event_flag, string> names_by_flag = {
      FSW_MAKE_PAIR_FROM_NAME(NoOp),
      FSW_MAKE_PAIR_FROM_NAME(PlatformSpecific),
      FSW_MAKE_PAIR_FROM_NAME(Created),
      FSW_MAKE_PAIR_FROM_NAME(Updated),
      FSW_MAKE_PAIR_FROM_NAME(Removed),
      FSW_MAKE_PAIR_FROM_NAME(Renamed),
      FSW_MAKE_PAIR_FROM_NAME(OwnerModified),
      FSW_MAKE_PAIR_FROM_NAME(AttributeModified),
      FSW_MAKE_PAIR_FROM_NAME(MovedFrom),
      FSW_MAKE_PAIR_FROM_NAME(MovedTo),
      FSW_MAKE_PAIR_FROM_NAME(IsFile),
      FSW_MAKE_PAIR_FROM_NAME(IsDir),
      FSW_MAKE_PAIR_FROM_NAME(IsSymLink),
      FSW_MAKE_PAIR_FROM_NAME(Link),
      FSW_MAKE_PAIR_FROM_NAME(Overflow)
    };
#undef FSW_MAKE_PAIR_FROM_NAME

    auto name = names_by_flag.find(flag);

    if (name == names_by_flag.end())
      throw libfsw_exception(_("Unknown event type."), FSW_ERR_UNKNOWN_VALUE);

    return name->second;
  }

  ostream& operator<<(ostream& out, const fsw_event_flag flag)
  {
    return out << event::get_event_flag_name(flag);
  }
}
