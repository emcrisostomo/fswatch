#                                               -*- Autoconf -*-
#
# Copyright (C) 2014, Enrico M. Crisostomo
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([fswatch], [0.0.3], [https://github.com/alandipert/fswatch/issues])
AC_REVISION([$Revision: 0.0.3-1 $])
AC_CONFIG_SRCDIR([fswatch.c])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AM_INIT_AUTOMAKE([-Wall -Werror gnu])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_HEADERS([CoreServices/CoreServices.h])
if ! test "x${ac_cv_header_CoreServices_CoreServices_h}" = "xyes" ; then
  AC_MSG_ERROR([Cannot find CoreServices/CoreServices.h])
fi

AM_CONDITIONAL([USE_CORESERVICES], [test "x${ac_cv_header_CoreServices_CoreServices_h}" = "xyes"])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK

AC_OUTPUT
