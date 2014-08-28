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
AC_INIT([fswatch], m4_esyscmd([./build-aux/git-generate-version.sh]), [enrico.m.crisostomo@gmail.com], [], [https://github.com/emcrisostomo/fswatch])
AC_COPYRIGHT([2014 (C) Enrico M. Crisostomo])
AC_REVISION([$Revision: m4_esyscmd([./build-aux/git-generate-version.sh])-m4_esyscmd([./build-aux/git-generate-revision.sh])$])
AC_CONFIG_SRCDIR([fswatch.cpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([scripts/fswatch-run-bash scripts/fswatch-run-zsh])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_MACRO_DIR([m4])

# Other packages
AC_CONFIG_SUBDIRS([libfswatch])

# Initialize Automake
AM_INIT_AUTOMAKE([-Wall -Werror gnu subdir-objects no-dist-gzip dist-zip])
AM_SILENT_RULES([yes])
AM_PROG_AR

# Initialize libtool 
LT_PREREQ([2.4.2])
LT_INIT
AC_SUBST([LIBTOOL_DEPS])

# Configure language
AC_LANG(C++)

# Checks for programs.
AC_PROG_CXX([clang++ g++])
AC_PROG_LN_S
AC_PATH_PROG([ZSH_PROG], [zsh], [no])
AC_PATH_PROG([BASH_PROG], [bash], [no])

if test "x${ZSH_PROG}" = "xno" && test "x${BASH_PROG}" = "xno" ; then
  AC_MSG_WARN([Either ZSH or the Bash shell is required by fswatch-run.])
fi

AM_CONDITIONAL([HAVE_ZSH], [test "x${ZSH_PROG}" != "xno"])
AM_CONDITIONAL([HAVE_BASH], [test "x${BASH_PROG}" != "xno"])

if test "x${ZSH_PROG}" = "xno" ; then
  ZSH_PATH=/bin/zsh
else
  ZSH_PATH=$ZSH_PROG
fi

if test "x${BASH_PROG}" = "xno" ; then
  BASH_PATH=/bin/bash
else
  BASH_PATH=$BASH_PROG
fi

AC_SUBST([ZSH_PATH])
AC_SUBST([BASH_PATH])

# Configure C++ compiler
AX_CXX_COMPILE_STDCXX_11(noext)
AX_CXXFLAGS_WARN_ALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h getopt.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_STRTOD
AC_CHECK_FUNCS([atexit], [], AC_MSG_ERROR([The atexit function cannot be found.]))
AC_CHECK_FUNCS([getopt_long])
AC_CHECK_FUNCS([realpath], [], AC_MSG_ERROR([The realpath function cannot be found.]))
AC_CHECK_FUNCS([regcomp])

# Variables used in man files
MAN_DATE=$(date +'%B %d, %Y')
AC_SUBST([MAN_DATE])

AC_CONFIG_FILES([man/fswatch.7])

AC_OUTPUT
