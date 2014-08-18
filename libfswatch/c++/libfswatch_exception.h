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
#ifndef LIBFSW_EXCEPTION_H
#  define LIBFSW_EXCEPTION_H

#  include "../c/error.h"
#  include <exception>
#  include <string>

namespace fsw
{

  class libfsw_exception : public std::exception
  {
  public:
    // TODO default code value should be taken from an error header
    libfsw_exception(std::string cause, int code = FSW_ERR_UNKNOWN_ERROR);
    virtual const char * what() const noexcept;
    virtual int error_code() const noexcept;
    virtual ~libfsw_exception() noexcept;

    explicit operator int() const noexcept;

  private:
    const std::string cause;
    const int code;
  };
}

#endif  /* LIBFSW_EXCEPTION_H */
