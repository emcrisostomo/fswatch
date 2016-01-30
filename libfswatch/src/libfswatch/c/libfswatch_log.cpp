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
#include "../c++/string/string_utils.hpp"
#include <cstdarg>

using namespace std;
using namespace fsw;

void fsw_log(const char *msg)
{
  if (fsw_is_verbose()) printf("%s", msg);
}

void fsw_flog(FILE *f, const char *msg)
{
  if (fsw_is_verbose()) fprintf(f, "%s", msg);
}

void fsw_logf(const char *format, ...)
{
  if (!fsw_is_verbose()) return;

  va_list args;
  va_start(args, format);

  vfprintf(stdout, format, args);

  va_end(args);
}

void fsw_flogf(FILE *f, const char *format, ...)
{
  if (!fsw_is_verbose()) return;

  va_list args;
  va_start(args, format);

  vfprintf(f, format, args);

  va_end(args);
}

void fsw_log_perror(const char *msg)
{
  if (fsw_is_verbose()) perror(msg);
}

void fsw_logf_perror(const char *format, ...)
{
  if (!fsw_is_verbose()) return;

  va_list args;
  va_start(args, format);

  perror(string_utils::vstring_from_format(format, args).c_str());

  va_end(args);
}
