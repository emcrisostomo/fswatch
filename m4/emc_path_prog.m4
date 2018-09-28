# -*- Autoconf -*-
#
# SYNOPSIS
#
#   EMC_PATH_PROG(variable, prog, [action-if-found], [action-if-not-found],
#   [variable-desc], [path])
#
# DESCRIPTION
#
#   Check whether prog exists in PATH, or in path if provided, and make variable
#   precious invoking AC_ARG_VAR(variable, variable-desc).  If it does, set
#   variable to its PATH, and execute action-if-found, otherwise unset variable
#   and execute action-if-not-found.  Both actions are optional.
#
# LICENSE
#
#   Copyright (c) 2015-2017 Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>
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
#serial 2

AC_DEFUN([EMC_PATH_PROG], [dnl
AC_ARG_VAR([$1], [$5])
AC_PATH_PROG([$1], [$2], [], [$6])
AS_VAR_SET_IF([ac_cv_path_$1],
  [$3],
  [
    AS_UNSET($1)
    $4
  ])
])dnl EMC_PATH_PROG
