# ===========================================================================
#     http://www.gnu.org/software/autoconf-archive/ax_cxx_have_thread_local.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CXX_HAVE_THREAD_LOCAL()
#
# DESCRIPTION
#
#   This macro checks if the thread_local storage specified, added in C++11, is
#   supported by the current compiler and libraries.
#
#   If it is, define the ax_cv_cxx_have_thread_local environment variable to
#   "yes" and define HAVE_CXX_THREAD_LOCAL.
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

AC_DEFUN([AX_CXX_HAVE_THREAD_LOCAL],
  [AC_CACHE_CHECK(
    [for thread_local storage specifier],
    ax_cv_cxx_have_thread_local,
    [dnl
      AC_LANG_PUSH([C++])
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
        [
          [using namespace std;]
          [static thread_local int x;]
        ],
        []
        )],
        [ax_cv_cxx_have_thread_local=yes],
        [ax_cv_cxx_have_thread_local=no]
      )
    AC_LANG_POP([C++])])
    if test x"$ax_cv_cxx_have_thread_local" = "xyes"
    then
      AC_DEFINE(HAVE_CXX_THREAD_LOCAL,
        1,
        [Define if the thread_local storage specified is available.])
    fi
  ])