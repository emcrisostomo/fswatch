# ===========================================================================
#       https://www.gnu.org/software/autoconf-archive/ax_prog_date.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_PROG_DATE()
#
# DESCRIPTION
#
#   This macro tries to determine the type of the date (1) command and some
#   of its non-standard capabilities.
#
#   The type is determined as follow:
#
#     * If the version string contains "GNU", then:
#       - The variable ax_cv_prog_date_gnu is set to "yes".
#       - The variable ax_cv_prog_date_type is set to "gnu".
#
#     * If date supports the "-v 1d" option, then:
#       - The variable ax_cv_prog_date_bsd is set to "yes".
#       - The variable ax_cv_prog_date_type is set to "bsd".
#
#     * If both previous checks fail, then:
#       - The variable ax_cv_prog_date_type is set to "unknown".
#
#   The following capabilities of GNU date are checked:
#
#     * If date supports the --date arg option, then:
#       - The variable ax_cv_prog_date_gnu_date is set to "yes".
#
#     * If date supports the --utc arg option, then:
#       - The variable ax_cv_prog_date_gnu_utc is set to "yes".
#
#   The following capabilities of BSD date are checked:
#
#     * If date supports the -v 1d  option, then:
#       - The variable ax_cv_prog_date_bsd_adjust is set to "yes".
#
#     * If date supports the -r arg option, then:
#       - The variable ax_cv_prog_date_bsd_date is set to "yes".
#
#   All the aforementioned variables are set to "no" before a check is
#   performed.
#
# LICENSE
#
#   Copyright (c) 2017 Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>
#
#   This program is free software: you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation, either version 3 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.

#serial 3

AC_DEFUN([AX_PROG_DATE], [dnl
  AC_CACHE_CHECK([for GNU date], [ax_cv_prog_date_gnu], [
    ax_cv_prog_date_gnu=no
    if date --version 2>/dev/null | head -1 | grep -q GNU
    then
      ax_cv_prog_date_gnu=yes
    fi
  ])
  AC_CACHE_CHECK([for BSD date], [ax_cv_prog_date_bsd], [
    ax_cv_prog_date_bsd=no
    if date -v 1d > /dev/null 2>&1
    then
      ax_cv_prog_date_bsd=yes
    fi
  ])
  AC_CACHE_CHECK([for date type], [ax_cv_prog_date_type], [
    ax_cv_prog_date_type=unknown
    if test "x${ax_cv_prog_date_gnu}" = "xyes"
    then
      ax_cv_prog_date_type=gnu
    elif test "x${ax_cv_prog_date_bsd}" = "xyes"
    then
      ax_cv_prog_date_type=bsd
    fi
  ])
  AS_VAR_IF([ax_cv_prog_date_gnu], [yes], [
    AC_CACHE_CHECK([whether GNU date supports --date], [ax_cv_prog_date_gnu_date], [
      ax_cv_prog_date_gnu_date=no
      if date --date=@1512031231 > /dev/null 2>&1
      then
        ax_cv_prog_date_gnu_date=yes
      fi
    ])
    AC_CACHE_CHECK([whether GNU date supports --utc], [ax_cv_prog_date_gnu_utc], [
      ax_cv_prog_date_gnu_utc=no
      if date --utc > /dev/null 2>&1
      then
        ax_cv_prog_date_gnu_utc=yes
      fi
    ])
  ])
  AS_VAR_IF([ax_cv_prog_date_bsd], [yes], [
    AC_CACHE_CHECK([whether BSD date supports -r], [ax_cv_prog_date_bsd_date], [
      ax_cv_prog_date_bsd_date=no
      if date -r 1512031231 > /dev/null 2>&1
      then
        ax_cv_prog_date_bsd_date=yes
      fi
    ])
  ])
    AS_VAR_IF([ax_cv_prog_date_bsd], [yes], [
    AC_CACHE_CHECK([whether BSD date supports -v], [ax_cv_prog_date_bsd_adjust], [
      ax_cv_prog_date_bsd_adjust=no
      if date -v 1d > /dev/null 2>&1
      then
        ax_cv_prog_date_bsd_adjust=yes
      fi
    ])
  ])
])dnl AX_PROG_DATE
