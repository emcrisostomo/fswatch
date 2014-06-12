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
#
AC_PREREQ([2.69])
AC_INIT([fswatch], [1.3.6], [enrico.m.crisostomo@gmail.com], [], [https://github.com/alandipert/fswatch])
AC_COPYRIGHT([2014 (C) Enrico M. Crisostomo])
AC_REVISION([$Revision: 1.3.6-1 $])
AC_CONFIG_SRCDIR([fswatch.cpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([-Wall -Werror gnu])

# Configure language
AC_LANG(C++)

# Checks for programs.
AC_PROG_CXX([clang++ g++])
AC_CHECK_PROG([HAVE_ZSH], [zsh], [yes])
AC_CHECK_PROG([HAVE_BASH], [bash], [yes])

AM_CONDITIONAL([HAVE_ZSH], [test "x${HAVE_ZSH}" = "xyes"])
AM_CONDITIONAL([HAVE_BASH], [test "x${HAVE_BASH}" = "xyes"])

if test "x${HAVE_ZSH}" != "xyes" && test "x${HAVE_BASH}" != "xyes" ; then
    AC_MSG_WARN([Either ZSH or the Bash shell is required by fswatch-run.])
fi

# Configure C++ compiler
AX_CXX_COMPILE_STDCXX_11(noext)
AX_CXXFLAGS_WARN_ALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h getopt.h fcntl.h sys/event.h sys/time.h])
AC_CHECK_HEADERS([CoreServices/CoreServices.h])
AC_CHECK_HEADERS([unordered_map unordered_set])
AC_CHECK_HEADERS([sys/inotify.h])

AM_CONDITIONAL([USE_CORESERVICES], [test "x${ac_cv_header_CoreServices_CoreServices_h}" = "xyes"])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_MODE_T

AC_CHECK_MEMBERS([struct stat.st_mtime],
   [],
   [],
   [
     AC_INCLUDES_DEFAULT
     #include <sys/stat.h>
   ])

AC_CHECK_MEMBERS([struct stat.st_mtimespec],
   [],
   [],
   [
     AC_INCLUDES_DEFAULT
     #include <sys/stat.h>
   ])

# Checks for library functions.
AC_FUNC_STRTOD
AC_CHECK_FUNCS([getopt_long])
AC_CHECK_FUNCS(
  [realpath],
  ,
  AC_MSG_ERROR([The realpath function cannot be found.])
)
AC_CHECK_FUNCS(
  [modf],
  ,
  AC_MSG_ERROR([The modf function cannot be found.])
)
AC_CHECK_FUNCS([regcomp])

AC_CHECK_DECLS(
  [kqueue, kevent],
  ,
  ,
  [
    AC_INCLUDES_DEFAULT
    [#include <sys/event.h>]
    [#include <sys/types.h>]
    [#include <sys/time.h>]
  ]
)

AC_OUTPUT
