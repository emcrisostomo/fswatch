# ===========================================================================
#     http://www.gnu.org/software/autoconf-archive/ax_fsevents_have_file_events.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_FSEVENTS_HAVE_FILE_EVENTS()
#
# DESCRIPTION
#
#   This macro checks if the macOS FSEvents API supports file events, and if it
#   supports the FSEventStreamSetDispatchQueue function.
#
#   If the FSEvents API is supported, the ax_cv_fsevents_have_file_events environment
#   variable is defined and set to "yes", and the HAVE_FSEVENTS_FILE_EVENTS symbol is
#   defined.
#
#   If the FSEventStreamSetDispatchQueue function is supported, the
#   ax_cv_fsevents_have_fseventstreamsetdispatchqueue environment variable is
#   defined and set to "yes", and the HAVE_FSEVENTS_FILE_EVENTS symbol is defined.
#
# CONTRIBUTING
#
#   You can contribute changes on GitHub by forking this repository:
#
#   https://github.com/emcrisostomo/CXX-Autoconf-Macros.git
#
# LICENSE
#
#   Copyright (c) 2014-2022 Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.  This file is offered as-is, without any
#   warranty.

#serial 2

AC_DEFUN([AX_FSEVENTS_HAVE_FILE_EVENTS],
  [AC_CACHE_CHECK(
    [for file events in macOS FSEvents API],
    ax_cv_fsevents_have_file_events,
    [dnl
      AC_LANG_PUSH([C])
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
        [#include <CoreServices/CoreServices.h>],
        [int i = kFSEventStreamCreateFlagFileEvents;]
        )],
        [ax_cv_fsevents_have_file_events=yes],
        [ax_cv_fsevents_have_file_events=no]
      )
      AC_LANG_POP([C])])
    AC_CACHE_CHECK(
      [for FSEventStreamSetDispatchQueue in macOS FSEvents API],
      ax_cv_fsevents_have_fseventstreamsetdispatchqueue,
      [dnl
        AC_LANG_PUSH([C])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
          [#include <CoreServices/CoreServices.h>],
          [FSEventStreamSetDispatchQueue(NULL, NULL);]
          )],
          [ax_cv_fsevents_have_fseventstreamsetdispatchqueue=yes],
          [ax_cv_fsevents_have_fseventstreamsetdispatchqueue=no]
        )
        AC_LANG_POP([C])])
    if test x"$ax_cv_fsevents_have_file_events" = "xyes"
    then
      AC_DEFINE(HAVE_FSEVENTS_FILE_EVENTS,
        1,
        [Define if the file events are supported by macOS FSEvents API.])
    fi
    if test x"$ax_cv_fsevents_have_fseventstreamsetdispatchqueue" = "xyes"
    then
      AC_DEFINE(HAVE_FSEVENTS_FSEVENTSTREAMSETDISPATCHQUEUE,
        1,
        [Define if the FSEventStreamSetDispatchQueue function is supported by macOS FSEvents API.])
    fi
  ])
