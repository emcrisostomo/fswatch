# ===========================================================================
#     http://www.gnu.org/software/autoconf-archive/ax_cxx_have_thread.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CXX_HAVE_THREAD()
#
# DESCRIPTION
#
#   This macro checks if std::thread, added in C++11, is defined in the
#   <thread> header.
#
#   If it is, define the ax_cv_cxx_have_thread environment variable to "yes"
#   and define HAVE_CXX_THREAD.
#
# CONTRIBUTING
#
#   You can contribute changes on GitHub by forking this repository:
#
#   https://github.com/emcrisostomo/CXX-Autoconf-Macros.git
#
# LICENSE
#
#   Copyright (c) 2014 Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.  This file is offered as-is, without any
#   warranty.

#serial 1

AC_DEFUN([AX_CXX_HAVE_THREAD],
  [AC_CACHE_CHECK(
    [for std::thread in thread],
    ax_cv_cxx_have_thread,
    [dnl
      AC_LANG_PUSH([C++])
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
        [
          [#include <thread>]
          [using namespace std;]
          [thread t;]
        ],
        []
        )],
        [ax_cv_cxx_have_thread=yes],
        [ax_cv_cxx_have_thread=no]
      )
    AC_LANG_POP([C++])])
    if test x"$ax_cv_cxx_have_thread" = "xyes"
    then
      AC_DEFINE(HAVE_CXX_THREAD,
        1,
        [Define if thread defines the std::thread class.])
    fi
  ])
