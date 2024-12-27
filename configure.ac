#                                               -*- Autoconf -*-
#
# Copyright (C) 2014-2022 Enrico M. Crisostomo
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Process this file with autoconf to produce a configure script.
#
dnl Copyright (C) 2014-2022 Enrico M. Crisostomo
AC_PREREQ([2.69])
m4_include([m4/fswatch_version.m4])
m4_include([m4/libfswatch_version.m4])
AC_INIT([fswatch], LIBFSWATCH_VERSION, [enrico.m.crisostomo@gmail.com], [], [https://github.com/emcrisostomo/fswatch])
AC_COPYRIGHT([2013-2022 (C) Enrico M. Crisostomo])
AC_REVISION([$Revision: LIBFSWATCH_REVISION$])
AC_CONFIG_SRCDIR([fswatch/src/fswatch.cpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([libfswatch/libfswatch_config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libfswatch/Makefile])
AC_CONFIG_FILES([libfswatch/doc/doxygen/Makefile])
AC_CONFIG_FILES([libfswatch/src/Makefile])
AC_CONFIG_FILES([libfswatch/src/libfswatch.pc])
AC_CONFIG_FILES([fswatch/Makefile fswatch/src/Makefile fswatch/doc/Makefile])
AC_CONFIG_FILES([po/Makefile.in])
AC_CONFIG_FILES([man/fswatch.7])
AC_CONFIG_MACRO_DIRS([m4])

# Compute the canonical target-system type variables
AC_CANONICAL_HOST
HOST_VENDOR=$host_vendor
HOST_OS=$host_os
AC_SUBST([HOST_VENDOR])
AC_SUBST([HOST_OS])

AC_CANONICAL_TARGET
TARGET_VENDOR=$target_vendor
TARGET_OS=$target_os
AC_SUBST([TARGET_VENDOR])
AC_SUBST([TARGET_OS])

# Initialize Automake.
AM_INIT_AUTOMAKE([-Wall -Werror gnu subdir-objects std-options 1.14])
AM_SILENT_RULES([yes])
AM_PROG_AR

# Configure language.
AC_LANG(C++)
AC_PROG_CXX([clang++ g++])

# Initialize libtool.
LT_PREREQ([2.4.2])
LT_INIT
AC_SUBST([LIBTOOL_DEPS])

# Configure package
AC_ARG_ENABLE([docker],
  [AS_HELP_STRING([--enable-docker], [enable docker (default=no)])],
  [use_docker=${enableval}],
  [use_docker=no])

# Check compiler
AX_COMPILER_VENDOR
AX_COMPILER_VERSION

# GCC 4.9 is the first compiler that implements <regex>: 4.8 shipped a broken
# implementation, so we have to explicitly check for a version >= 4.9
AS_VAR_IF([ax_cv_cxx_compiler_vendor], [gnu], [dnl
  AX_COMPARE_VERSION([${ax_cv_cxx_compiler_version}], [lt], [4.9], [AC_MSG_ERROR([GCC v. 4.9 is required])])
])

# OS-specific treatment
AM_CONDITIONAL([OS_CYGWIN], [test "x${target_os}" = "xcygwin"])

# Set library interface version
AC_SUBST([AM_LIBFSWATCH_API_VERSION], LIBFSWATCH_API_VERSION)

# Checks for programs.
EMC_PATH_PROG([DOXYGEN],  [doxygen],  [], [AC_MSG_WARN([Doxygen support disabled])],  [doxygen path])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "${DOXYGEN}"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([libfswatch/doc/doxygen/Doxyfile])])

EMC_PATH_PROG([MAKEINFO_PROG], [makeinfo], [], [AC_MSG_WARN([Makeinfo support disabled])], [makeinfo path])
AM_CONDITIONAL([HAVE_MAKEINFO], [test -n "${MAKEINFO_PROG}"])

# Initialize gettext.
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.22.5])
AM_CONDITIONAL([USE_NLS], [test "x${USE_NLS}" = "xyes"])

# Configure C++ compiler
AX_CXX_COMPILE_STDCXX_17
AX_CXXFLAGS_WARN_ALL

# Checks for libraries.
AC_SEARCH_LIBS([pthread_create], [pthread])

# Check for optional header files.
AC_CHECK_HEADERS([unordered_map unordered_set])
AC_CHECK_HEADERS([getopt.h])

# Check for optional header files required to support OS-specific monitors

# Check for macOS
AS_CASE([${HOST_OS}], ["darwin"*], [
    IS_DARWIN=true
    DARWIN_VER="${HOST_OS#darwin}"
    AC_SUBST([IS_DARWIN])
    AC_SUBST([DARWIN_VER])
])

AS_VAR_SET_IF([IS_DARWIN], [
  AX_COMPARE_VERSION([${DARWIN_VER}], [ge], [9.0],  [AC_DEFINE([HAVE_MACOS_GE_10_5],  [1], ["macOS is at least the specified version"])])
  AX_COMPARE_VERSION([${DARWIN_VER}], [ge], [11.0], [AC_DEFINE([HAVE_MACOS_GE_10_7],  [1], ["macOS is at least the specified version"])])
  AX_COMPARE_VERSION([${DARWIN_VER}], [ge], [13.0], [AC_DEFINE([HAVE_MACOS_GE_10_9],  [1], ["macOS is at least the specified version"])])
  AX_COMPARE_VERSION([${DARWIN_VER}], [ge], [14.0], [AC_DEFINE([HAVE_MACOS_GE_10_10], [1], ["macOS is at least the specified version"])])
  AX_COMPARE_VERSION([${DARWIN_VER}], [ge], [17.0], [AC_DEFINE([HAVE_MACOS_GE_10_13], [1], ["macOS is at least the specified version"])])
  # Check for macOS FSEvents.  FSEvents added file events in a later release, so
  # this check must be performed separately.
  AC_CHECK_HEADERS([CoreServices/CoreServices.h])
  AS_VAR_IF([ac_cv_header_CoreServices_CoreServices_h], ["yes"], [AX_FSEVENTS_HAVE_FILE_EVENTS])
])

AM_CONDITIONAL([USE_FSEVENTS], [test "x${ax_cv_fsevents_have_fseventstreamsetdispatchqueue}" = "xyes"])

# Check for kqueue: if the sys/event.h header is present, perform a check for
# the kqueue and kevent functions.
AC_CHECK_HEADERS([sys/event.h])
AS_VAR_IF([ac_cv_header_sys_event_h], ["yes"], [
  AS_VAR_SET([KQUEUE_AVAILABLE], ["yes"])
  AC_CHECK_DECLS(
    [kqueue, kevent],
    [],
    [AS_VAR_SET([KQUEUE_AVAILABLE], ["no"])],
    [
      AC_INCLUDES_DEFAULT
      [#include <sys/event.h>]
      [#include <sys/types.h>]
      [#include <sys/time.h>]
    ]
  )
  AC_CHECK_LIB([c], [kqueue], [], [AS_VAR_SET([KQUEUE_AVAILABLE], ["no"])])
])

AM_CONDITIONAL([USE_KQUEUE], [test "x${KQUEUE_AVAILABLE}" = "xyes"])

# Check for Solaris/Illumos File Events Notification API.
AC_CHECK_HEADERS([port.h])
AS_VAR_IF([ac_cv_header_port_h], ["yes"], [
  AS_VAR_SET([FEN_AVAILABLE], ["yes"])
  AC_CHECK_DECLS(
    [port_create],
    [],
    [AS_VAR_SET([FEN_AVAILABLE], ["no"])],
    [
      AC_INCLUDES_DEFAULT
      [#include <port.h>]
    ]
  )
  AC_CHECK_LIB([c], [port_create], [], [AS_VAR_SET([FEN_AVAILABLE], ["no"])])
])

AM_CONDITIONAL([USE_FEN], [test "x${FEN_AVAILABLE}" = "xyes"])

# Check for Linux inotify.
AC_CHECK_HEADERS([sys/inotify.h])
AS_VAR_IF([ac_cv_header_sys_inotify_h], ["yes"], [
  AS_VAR_SET([INOTIFY_AVAILABLE], ["yes"])
  AC_CHECK_DECLS(
    [inotify_init, inotify_add_watch, inotify_rm_watch],
    [],
    [AS_VAR_SET([INOTIFY_AVAILABLE], ["no"])],
    [
      AC_INCLUDES_DEFAULT
      [#include <sys/inotify.h>]
    ]
  )
  AC_CHECK_LIB([c], [inotify_init], [], [AS_VAR_SET([INOTIFY_AVAILABLE], ["no"])])
])

AM_CONDITIONAL([USE_INOTIFY],  [test "x${INOTIFY_AVAILABLE}" = "xyes"])

# Check for Microsoft Windows directory change notification API
AS_VAR_SET([WINDOWS_AVAILABLE], ["yes"])
AC_CHECK_HEADERS([windows.h], [], [AS_VAR_SET([WINDOWS_AVAILABLE], ["no"])])
AS_VAR_IF([WINDOWS_AVAILABLE], ["yes"], [
  AC_CHECK_DECLS(
    [FindFirstChangeNotification, FindNextChangeNotification, FindCloseChangeNotification, WaitForSingleObject, ReadDirectoryChangesW],
    [],
    [AS_VAR_SET([WINDOWS_AVAILABLE], ["no"])],
    [
      AC_INCLUDES_DEFAULT
      [#include <windows.h>]
    ]
  )
])

AM_CONDITIONAL([USE_WINDOWS], [test "x${WINDOWS_AVAILABLE}" = "xyes"])
AS_VAR_IF([WINDOWS_AVAILABLE], ["yes"], [AC_DEFINE([HAVE_WINDOWS], [1], [Windows API present.])])

# Check for CygWin only if Windows is available
AS_VAR_IF([WINDOWS_AVAILABLE], ["yes"], [
  AS_VAR_SET([CYGWIN_AVAILABLE], ["yes"])
  AC_CHECK_HEADERS([sys/cygwin.h], [], [AS_VAR_SET([CYGWIN_AVAILABLE], ["no"])])
  AS_VAR_IF([CYGWIN_AVAILABLE], ["yes"], [
    AC_CHECK_DECLS(
      [cygwin_create_path],
      [],
      [AS_VAR_SET([CYGWIN_AVAILABLE], ["no"])],
      [
        AC_INCLUDES_DEFAULT
        [#include <sys/cygwin.h>]
      ]
    )
  ])
])

AM_CONDITIONAL([USE_CYGWIN], [test "x${CYGWIN_AVAILABLE}" = "xyes"])
AS_VAR_IF([CYGWIN_AVAILABLE], ["yes"], [AC_DEFINE([HAVE_CYGWIN], [1], [CygWin API present.])])

# Checks for typedefs, structures, and compiler characteristics.
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
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK

# Check if realpath is available: if it is not and the host OS is Windows, then
# build its implementation, otherwise fail
AC_CHECK_FUNCS([realpath], [AS_VAR_SET([HAVE_REALPATH], ["yes"])], [AS_VAR_SET([HAVE_REALPATH], ["no"])])
AS_VAR_IF([HAVE_REALPATH],
  ["no"],
  [AS_VAR_IF([WINDOWS_AVAILABLE],
    ["yes"],
    [dnl
      AS_VAR_SET([BUILD_WIN_REALPATH], ["yes"])
    ],
    [AC_MSG_ERROR([The realpath function cannot be found.])])
  ]
)

AM_CONDITIONAL([BUILD_WIN_REALPATH], [test "x${BUILD_WIN_REALPATH}" = "xyes"])
AS_VAR_IF([HAVE_REALPATH], ["yes"], [AC_DEFINE([HAVE_REALPATH], [1], [Realpath present.])])

# Check for optional library functions.
AS_VAR_IF([ac_cv_header_getopt_h], ["yes"], [AC_CHECK_FUNCS([getopt_long])])

# Create Dockerfile for test build containers
AS_VAR_IF([use_docker], [yes],
  [
    AX_GIT_CURRENT_BRANCH
    AC_CONFIG_FILES([docker/alpine/Dockerfile])
    AC_CONFIG_FILES([docker/alpine-no-gettext/Dockerfile])
    AC_CONFIG_FILES([docker/debian10/Dockerfile])
    AC_CONFIG_FILES([docker/debian11/Dockerfile])
    AC_CONFIG_FILES([docker/debian-stable/Dockerfile])
    AC_CONFIG_FILES([docker/debian-testing/Dockerfile])
  ],
  [])

# Variables used in man files.
AX_PROG_DATE

if test -n "${SOURCE_DATE_EPOCH}"
then
  AS_VAR_IF([ax_cv_prog_date_gnu], [yes], [MAN_DATE=$(LC_ALL=C        date --utc --date=@${SOURCE_DATE_EPOCH} +'%B %d, %Y')])
  AS_VAR_IF([ax_cv_prog_date_bsd], [yes], [MAN_DATE=$(LC_ALL=C TZ=UTC date            -r ${SOURCE_DATE_EPOCH} +'%B %d, %Y')])
else
  MAN_DATE=$(date +'%B %d, %Y')
fi

FSWATCH=${PACKAGE_NAME}
MAN_BUG_REPORT=${PACKAGE_BUGREPORT}

AC_SUBST([MAN_DATE])
AC_SUBST([FSWATCH])
AC_SUBST([MAN_BUG_REPORT])

AC_OUTPUT
