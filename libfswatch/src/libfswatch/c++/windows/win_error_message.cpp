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
#  include "win_error_message.hpp"
#  include "../../c/libfswatch_log.h"

using namespace std;

namespace fsw
{
  win_error_message win_error_message::current()
  {
    return win_error_message();
  }

  win_error_message::win_error_message(DWORD error_code) : err_code{error_code}{}
  win_error_message::win_error_message() : err_code{GetLastError()}{}

  DWORD win_error_message::get_error_code() const
  {
    return err_code;
  }

  wstring win_error_message::get_message() const
  {
    if (initialized) return msg;
    initialized = true;

    LPWSTR buf = nullptr;
    DWORD ret_size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                    NULL,
                                    err_code,
                                    0,
                                    (LPWSTR)&buf,
                                    0,
                                    nullptr);

    if (ret_size > 0)
    {
      msg = buf;
      LocalFree(buf);
    }
    else
    {
      msg = L"The system error message could not be formatted.";
    }

    return msg;
  }

  win_error_message::operator wstring() const { return get_message(); }
}

#endif  /* HAVE_WINDOWS */
