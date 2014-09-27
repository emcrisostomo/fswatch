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
dnl Copyright (C) 2014, Enrico M. Crisostomo
AC_PREREQ([2.69])
m4_include([m4/libfswatch_version.m4])
AC_INIT([libfswatch], LIBFSWATCH_VERSION, [enrico.m.crisostomo@gmail.com], [], [https://github.com/emcrisostomo/fswatch])
AC_COPYRIGHT([2014 (C) Enrico M. Crisostomo])
AC_REVISION([$Revision: LIBFSWATCH_REVISION$])
AC_CONFIG_SRCDIR([src/libfswatch/c/libfswatch.cpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([libfswatch_config.h])
AC_CONFIG_FILES([Makefile src/Makefile src/libfswatch/Makefile po/Makefile.in])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_MACRO_DIR([m4])

# Initialize Automake
AM_INIT_AUTOMAKE([-Wall -Werror gnu subdir-objects])
AM_SILENT_RULES([yes])
AM_PROG_AR

# Initialize libtool 
LT_PREREQ([2.4.2])
LT_INIT
AC_SUBST([LIBTOOL_DEPS])

# Set library interface version
AC_SUBST([AM_LIBFSWATCH_API_VERSION], LIBFSWATCH_API_VERSION)

# Configure language
AC_LANG(C++)
AC_PROG_CXX([clang++ g++])

# Checks for programs.

# Initialize gettext
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.2])

# Configure C++ compiler
AX_CXX_COMPILE_STDCXX_11(noext)
AX_CXXFLAGS_WARN_ALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h fcntl.h])
AC_CHECK_HEADERS([sys/event.h sys/inotify.h])
AC_CHECK_HEADERS([CoreServices/CoreServices.h])
AC_CHECK_HEADERS([unordered_map unordered_set])

AM_CONDITIONAL([USE_KQUEUE], [test "x${ac_cv_header_sys_event_h}" = "xyes"])
AM_CONDITIONAL([USE_INOTIFY], [test "x${ac_cv_header_sys_inotify_h}" = "xyes"])

if test "x${ac_cv_header_CoreServices_CoreServices_h}" = "xyes" ; then
  AX_FSEVENTS_HAVE_FILE_EVENTS
fi

AM_CONDITIONAL([USE_FSEVENTS], [test "x${ax_cv_fsevents_have_file_events}" = "xyes"])

# Some compilers declare to conform to C++11 but the library does not.
# These OS X versions are known to miss at least the listed headers:
#   - 10.6 Snow Leopard: mutex, atomic
#   - 10.7 Lion: mutex, atomic
#   - 10.8 Mountain Lion: mutex, atomic
# Let's check for <mutex> and skip multi-threading code if not found.
AC_CHECK_HEADER([mutex])

if test "x${ac_cv_header_mutex}" = "xyes" ; then
  AC_DEFINE(HAVE_CXX_MUTEX,
        1,
        [Define if <mutex> is available.])
else
    AC_WARN(AC_PACKAGE_NAME [is not thread-safe because required C++11 library classes are not available.].)
fi

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

AC_CHECK_TYPE([std::unique_ptr<std::string>],
  [AC_DEFINE(HAVE_CXX_UNIQUE_PTR,
        1,
        [Define if std::unique_ptr in <memory> is available.])],
  [],
  [[#include <memory>]])

# Checks for library functions.
AC_CHECK_FUNCS([realpath], [], AC_MSG_ERROR([The realpath function cannot be found.]))
AC_CHECK_FUNCS([modf], [], AC_MSG_ERROR([The modf function cannot be found.]))
AC_CHECK_FUNCS([select], [], AC_MSG_ERROR([The select function cannot be found.]))
AC_CHECK_FUNCS([regcomp])

AC_CHECK_DECLS(
  [kqueue, kevent],
  [],
  [],
  [
    AC_INCLUDES_DEFAULT
    [#include <sys/event.h>]
    [#include <sys/types.h>]
    [#include <sys/time.h>]
  ]
)

AX_CXX_HAVE_THREAD_LOCAL

if ! test "x${ax_cv_cxx_have_thread_local}" = "xyes" ; then
  AC_WARN(AC_PACKAGE_NAME [is missing some functions because the current combination of compiler and libraries do not support the thread_local storage specifier].)
fi

AC_OUTPUT
