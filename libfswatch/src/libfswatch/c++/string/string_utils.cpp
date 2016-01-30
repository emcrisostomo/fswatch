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
#include <cstdarg>
#include <cstdio>
#include <vector>
#include "string_utils.hpp"

using namespace std;

namespace fsw
{
  namespace string_utils
  {
    string vstring_from_format(const char *format, va_list args)
    {
      size_t current_buffer_size = 0;
      int required_chars = 512;

      vector<char> buffer;

      do
      {
        current_buffer_size += required_chars;
        buffer.resize(current_buffer_size);
        required_chars = vsnprintf(&buffer[0], current_buffer_size, format, args);

        // If an encoding error occurs, break and write an empty string into the
        // buffer.
        if (required_chars < 0)
        {
          buffer.resize(1);
          break;
        }
      }
      while ((size_t) required_chars > current_buffer_size);

      return string(&buffer[0]);
    }

    string string_from_format(const char *format, ...)
    {
      va_list args;
      va_start(args, format);

      string ret = vstring_from_format(format, args);

      va_end(args);

      return ret;
    }
  }
}