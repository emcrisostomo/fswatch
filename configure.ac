#                                               -*- Autoconf -*-
#
# Copyright (C) 2014, 2015, Enrico M. Crisostomo
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
dnl Copyright (C) 2014, 2015, Enrico M. Crisostomo
AC_PREREQ([2.69])
m4_include([m4/fswatch_version.m4])
AC_INIT([fswatch], FSWATCH_VERSION, [enrico.m.crisostomo@gmail.com], [], [https://github.com/emcrisostomo/fswatch])
AC_COPYRIGHT([2014 (C) Enrico M. Crisostomo])
AC_REVISION([$Revision: FSWATCH_REVISION$])
AC_CONFIG_SRCDIR([src/fswatch.cpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile po/Makefile.in])
AC_CONFIG_FILES([scripts/fswatch-run-bash scripts/fswatch-run-zsh])
AC_CONFIG_FILES([build-aux/create-pot-template.sh])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_MACRO_DIR([m4])

# Compute the canonical target-system type variables
AC_CANONICAL_TARGET
TARGET_VENDOR=$target_vendor
TARGET_OS=$target_os
AC_SUBST([TARGET_VENDOR])
AC_SUBST([TARGET_OS])

# Other packages
AC_CONFIG_SUBDIRS([libfswatch])

# Initialize Automake
AM_INIT_AUTOMAKE([-Wall -Werror gnu subdir-objects])
AM_SILENT_RULES([yes])
AM_PROG_AR

# Initialize libtool 
LT_PREREQ([2.4.2])
LT_INIT
AC_SUBST([LIBTOOL_DEPS])

# Configure language
AC_LANG(C++)
AC_PROG_CXX([clang++ g++])

# Initialize gettext
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.2])

# Checks for programs.
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
FSWATCH=${PACKAGE_NAME}
MAN_BUG_REPORT=${PACKAGE_BUGREPORT}
AC_SUBST([MAN_DATE])
AC_SUBST([FSWATCH])
AC_SUBST([MAN_BUG_REPORT])

# Variables used in gettext management scripts
PO_BUG_REPORT=${PACKAGE_BUGREPORT}
PO_PACKAGE_NAME=${PACKAGE_NAME}
PO_PACKAGE_VERSION=${PACKAGE_VERSION}
AC_SUBST([PO_BUG_REPORT])
AC_SUBST([PO_PACKAGE_NAME])
AC_SUBST([PO_PACKAGE_VERSION])

AC_CONFIG_FILES([man/fswatch.7])

AC_OUTPUT
