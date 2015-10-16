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
#include "libfswatch.h"
#include "libfswatch_log.h"
#include <cstdarg>
#include <cstdio>
#include <vector>
#include <iostream>

using namespace std;

void fsw_log(const char * msg)
{
  if (fsw_is_verbose()) printf("%s", msg);
}

void fsw_flog(FILE * f, const char * msg)
{
  if (fsw_is_verbose()) fprintf(f, "%s", msg);
}

void fsw_logf(const char * format, ...)
{
  if (!fsw_is_verbose()) return;

  va_list args;
  va_start(args, format);

  vfprintf(stdout, format, args);

  va_end(args);
}

void fsw_flogf(FILE * f, const char * format, ...)
{
  if (!fsw_is_verbose()) return;

  va_list args;
  va_start(args, format);

  vfprintf(f, format, args);

  va_end(args);
}

void fsw_log_perror(const char * msg)
{
  if (fsw_is_verbose()) perror(msg);
}

void fsw_logf_perror(const char * format, ...)
{
  if (!fsw_is_verbose()) return;

  size_t current_buffer_size = 0;
  int required_chars = 512;

  vector<char> buffer;

  do
  {
    va_list args;
    va_start(args, format);

    current_buffer_size += required_chars;
    buffer.resize(current_buffer_size);
    required_chars = vsnprintf(&buffer[0], current_buffer_size, format, args);

    va_end(args);

    // If an encoding error occurs, break and write an empty string into the
    // buffer.
    if (required_chars < 0)
    {
      buffer.resize(1);
      break;
    }
  }
  while (required_chars > current_buffer_size);

  perror(&buffer[0]);
}
