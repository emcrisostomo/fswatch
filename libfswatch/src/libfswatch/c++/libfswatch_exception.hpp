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
 * @brief Base exception of the `libfswatch` library.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef LIBFSW_EXCEPTION_H
#  define LIBFSW_EXCEPTION_H

#  include "../c/error.h"
#  include <exception>
#  include <string>

namespace fsw
{
  /**
   * @brief Base exception of the `libfswatch` library.
   *
   * An instance of this class stores an error message and an integer error
   * code.
   */
  class libfsw_exception : public std::exception
  {
  public:
    /**
     * @brief Constructs an exception with the specified @p cause and error
     * @p code.
     *
     * @param cause The error message.
     * @param code The error code.
     */
    libfsw_exception(std::string cause, int code = FSW_ERR_UNKNOWN_ERROR);

    libfsw_exception( const libfsw_exception& other ) noexcept;

    libfsw_exception& operator=(const libfsw_exception&) noexcept;

    /**
     * @brief Gets the error message.
     *
     * @return The error message.
     */
    virtual const char *what() const noexcept;

    /**
     * @brief Gets the error code.
     *
     * @return The error code.
     */
    virtual int error_code() const noexcept;

    /**
     * @brief Destructs an instance of this class.
     */
    virtual ~libfsw_exception() noexcept;

    /**
     * @brief Gets the error code.
     */
    explicit operator int() const noexcept;

  private:
    std::string cause;
    int code;
  };
}

#endif  /* LIBFSW_EXCEPTION_H */
