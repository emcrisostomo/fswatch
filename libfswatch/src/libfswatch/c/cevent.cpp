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
#ifdef HAVE_CONFIG_H
#  include "libfswatch_config.h"
#endif

#include "cevent.h"
#include "../c++/event.h"

using namespace std;
using namespace fsw;

fsw_event_flag fsw_get_event_flag_by_name(const char * name)
{
  return event::get_event_flag_by_name(name);
}

char * fsw_get_event_flag_name(const fsw_event_flag flag)
{
  string name = event::get_event_flag_name(flag);
  char * cstr = new char[name.size() + 1];
  strcpy(cstr, name.c_str());
  
  return cstr;
}