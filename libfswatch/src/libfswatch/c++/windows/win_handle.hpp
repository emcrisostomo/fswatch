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
/**
 * @file
 * @brief Header of the fsw::win_handle class.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */
#ifndef FSW_WINDOWS_HANDLE_H
#  define FSW_WINDOWS_HANDLE_H

#  include <windows.h>

namespace fsw
{
  /**
   * @brief A RAII wrapper around Microsoft Windows `HANDLE`.
   *
   * This class is a movable, non-copyable RAII wrapper on `HANDLE`.
   */
  class win_handle
  {
  public:
    /**
     * @brief Checks whether @p handle is valid.
     *
     * A @p handle is valid is if its value is not `null` and if is not
     * `INVALID_HANDLE_VALUE`.
     *
     * @param handle The handle to check.
     * @return Returns @c true if @p handle is valid, @c false otherwise.
     */
    static bool is_valid(const HANDLE & handle);

    /**
     * @brief Constructs an instance wrapping `INVALID_HANDLE_VALUE`.
     */
    win_handle();

    /**
     * @brief Constructs an instance wrapping @p handle.
     */
    win_handle(HANDLE handle);

    /**
     * @brief Destructs a handle.
     *
     * If the handle is valid (is_valid()) it is closed invoking
     * `CloseHandle()`.
     *
     * @see is_valid(const HANDLE &)
     */
    virtual ~win_handle();

    /**
     * @brief Returns the handle value as `HANDLE` instance.
     */
    operator HANDLE() const;

    /**
     * @brief Checks whether the handle is valid.
     *
     * @return Returns @c true if the handle is valid, @c false otherwise.
     * @see is_valid()
     */
    bool is_valid() const;

    /**
     * @brief Deleted copy constructor.
     */
    win_handle(const win_handle&) = delete;

    /**
     * @brief Deleted copy assignment operator.
     */
    win_handle& operator=(const win_handle&) = delete;

    /**
     * @brief Move constructor.
     *
     * The move constructors moves the handle value wrapped by @p other to the
     * target instance.  The handle value in @p other is set to
     * `INVALID_HANDLE_VALUE`.  The previously wrapped instance is closed
     * invoking `CloseHandle` if it is valid.
     *
     * @param other The handle to move.
     */
    win_handle(win_handle&& other) noexcept;

    /**
     * @brief Move assignment operator.
     *
     * The move assignment operator moves the handle value wrapped by @p other
     * to the target instance.  The handle value in @p other is set to
     * `INVALID_HANDLE_VALUE`.  The previously wrapped instance is closed
     * invoking `CloseHandle` if it is valid.
     *
     * @param other The handle to move.
     */
    win_handle& operator=(win_handle&& other) noexcept;

    /**
     * @brief Assigns a @p handle to the current instance.
     *
     * The previously wrapped instance is closed invoking `CloseHandle` if it is
     * valid.
     *
     * @param handle The handle value to assign to the current instance.
     */
    win_handle& operator=(const HANDLE& handle);
  private:
    HANDLE h;
  };
}

#endif  /* FSW_WINDOWS_HANDLE_H */
