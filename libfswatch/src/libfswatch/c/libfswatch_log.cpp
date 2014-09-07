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
#include "libfswatch.h"
#include "libfswatch_log.h"
#include <iostream>

using namespace std;

void libfsw_log(const char * msg)
{
  if (fsw_is_verbose())
  {
    cout << msg;
  }
}

void libfsw_perror(const char * msg)
{
  // TODO
  if (fsw_is_verbose())
  {
    perror(msg);
  }
}
