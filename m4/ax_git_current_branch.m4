# -*- Autoconf -*-
#
# SYNOPSIS
#
#   AX_GIT_CURRENT_BRANCH()
#
# DESCRIPTION
#
#   Sets the ax_git_current_branch variable to the current git branch, if any,
#   otherwise it lets it unset.
#
# LICENSE
#
#   Copyright (c) 2016-2017 Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>
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

AC_DEFUN_ONCE([AX_GIT_CURRENT_BRANCH],[dnl
EMC_PATH_PROG([GIT], [git], [], [], [git path])
AS_VAR_SET_IF([GIT],
  [
    AC_MSG_CHECKING([for current git branch])
    AS_VAR_SET([ax_git_current_branch], [$("${GIT}" rev-parse --symbolic-full-name --abbrev-ref HEAD)])
    AS_IF(dnl
      [test $? -eq 0],
      [AC_MSG_RESULT([${ax_git_current_branch}])],
      [
        AC_MSG_RESULT([])
        AC_MSG_WARN([An error occurred while invoking git])
        AS_UNSET([ax_git_current_branch])
      ])
    AC_SUBST([ax_git_current_branch])
  ], [])
])dnl AX_GIT_CURRENT_BRANCH
