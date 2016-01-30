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

#ifdef HAVE_WINDOWS

#  include "gettext_defs.h"
#  include "win_handle.hpp"
#  include "../../c/libfswatch_log.h"

using namespace std;

namespace fsw
{
  bool win_handle::is_valid(const HANDLE & handle)
  {
    return (handle != INVALID_HANDLE_VALUE && handle != nullptr);
  }

  win_handle::win_handle() : h(INVALID_HANDLE_VALUE){}
  win_handle::win_handle(HANDLE handle) : h(handle){}

  win_handle::~win_handle()
  {
    if (is_valid())
    {
      FSW_ELOGF(_("Closing handle: %d.\n"), h);
      CloseHandle(h);
    }
  }

  win_handle::operator HANDLE() const { return h; }

  bool win_handle::is_valid() const
  {
    return win_handle::is_valid(h);
  }

  win_handle& win_handle::operator=(const HANDLE& handle)
  {
    if (is_valid() && h != handle) CloseHandle(h);

    h = handle;

    return *this;
  }

  win_handle::win_handle(win_handle&& other) noexcept
  {
    h = other.h;
    other.h = INVALID_HANDLE_VALUE;
  }

  win_handle& win_handle::operator=(win_handle&& other) noexcept
  {
    if (this == &other) return *this;

    if (is_valid() && h != other.h) CloseHandle(h);

    h = other.h;
    other.h = INVALID_HANDLE_VALUE;

    return *this;
  }
}

#endif  /* HAVE_WINDOWS */
