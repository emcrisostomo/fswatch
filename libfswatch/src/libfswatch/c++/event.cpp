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
#include "event.h"
#include "gettext_defs.h"
#include "libfswatch_exception.h"
#include <map>

using namespace std;
using namespace fsw;

#define FSW_MAKE_PAIR_FROM_NAME(p) {p, #p}
static const std::map<fsw_event_flag, std::string> names_by_flag = {
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
  FSW_MAKE_PAIR_FROM_NAME(Link)
};
#undef FSW_MAKE_PAIR_FROM_NAME

#define FSW_MAKE_PAIR_FROM_NAME(p) {#p, p}
static const std::map<std::string, fsw_event_flag> flag_by_names = {
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
  FSW_MAKE_PAIR_FROM_NAME(Link)
};
#undef FSW_MAKE_PAIR_FROM_NAME

event::event(string path, time_t evt_time, vector<fsw_event_flag> flags) :
  path(path), evt_time(evt_time), evt_flags(flags)
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

fsw_event_flag event::get_event_flag_by_name(const std::string &name)
{
  auto flag = flag_by_names.find(name);

  if (flag == flag_by_names.end())
    throw libfsw_exception(_("Unknown event type: ") + name, FSW_ERR_UNKNOWN_VALUE);

  return flag->second;
}

std::string event::get_event_flag_name(const fsw_event_flag &flag)
{
  auto name = names_by_flag.find(flag);

  if (name == names_by_flag.end())
    throw libfsw_exception(_("Unknown event type."), FSW_ERR_UNKNOWN_VALUE);

  return name->second;
}

std::ostream& operator<<(std::ostream& out, const fsw_event_flag flag)
{
  const char* s = 0;
#define FSW_PROCESS_ENUM_VAL(p) case(p): s = #p; break;
  switch (flag)
  {
    FSW_PROCESS_ENUM_VAL(NoOp);
    FSW_PROCESS_ENUM_VAL(PlatformSpecific);
    FSW_PROCESS_ENUM_VAL(Created);
    FSW_PROCESS_ENUM_VAL(Updated);
    FSW_PROCESS_ENUM_VAL(Removed);
    FSW_PROCESS_ENUM_VAL(Renamed);
    FSW_PROCESS_ENUM_VAL(OwnerModified);
    FSW_PROCESS_ENUM_VAL(AttributeModified);
    FSW_PROCESS_ENUM_VAL(MovedFrom);
    FSW_PROCESS_ENUM_VAL(MovedTo);
    FSW_PROCESS_ENUM_VAL(IsFile);
    FSW_PROCESS_ENUM_VAL(IsDir);
    FSW_PROCESS_ENUM_VAL(IsSymLink);
    FSW_PROCESS_ENUM_VAL(Link);
  }
#undef FSW_PROCESS_ENUM_VAL

  return out << s;
}