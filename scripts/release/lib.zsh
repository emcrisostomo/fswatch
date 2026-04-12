#!/bin/zsh
# -*- vim:fenc=utf-8:et:sw=2:ts=2:sts=2
#
# Copyright (c) 2026 Enrico M. Crisostomo
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

setopt local_options
setopt local_traps
unsetopt glob_subst

set -o errexit
set -o nounset

RELEASE_REPO=${RELEASE_REPO:-emcrisostomo/fswatch}

release_die()
{
  print -u2 -- "error: $*"
  exit 1
}

release_warn()
{
  print -u2 -- "warning: $*"
}

release_info()
{
  print -- "$*"
}

release_repo_root()
{
  git rev-parse --show-toplevel 2> /dev/null ||
    release_die "not inside a git repository"
}

release_parse_version()
{
  (( $# == 1 )) || release_die "release_parse_version expects one argument"

  RELEASE_VERSION=$1
  if [[ "${RELEASE_VERSION}" =~ '^([0-9]+[.][0-9]+[.][0-9]+)(-rc[0-9]+|-develop)?$' ]]
  then
    RELEASE_BASE_VERSION=${match[1]}
    RELEASE_VERSION_MODIFIER=${match[2]:-}
  else
    release_die "invalid version '${RELEASE_VERSION}'; expected X.Y.Z, X.Y.Z-rcN, or X.Y.Z-develop"
  fi

  RELEASE_IS_DEVELOP=0
  RELEASE_IS_PRERELEASE=0

  if [[ "${RELEASE_VERSION_MODIFIER}" == "-develop" ]]
  then
    RELEASE_IS_DEVELOP=1
  elif [[ "${RELEASE_VERSION_MODIFIER}" == -rc<-> ]]
  then
    RELEASE_IS_PRERELEASE=1
  elif [[ -n "${RELEASE_VERSION_MODIFIER}" ]]
  then
    release_die "unsupported version modifier '${RELEASE_VERSION_MODIFIER}'"
  fi
}

release_m4_define()
{
  (( $# == 2 )) || release_die "release_m4_define expects file and macro name"
  local file=$1
  local macro=$2

  sed -n "s/^m4_define(\\[${macro}\\], \\[\\([^]]*\\)\\])$/\\1/p" "${file}" |
    tail -n 1
}

release_cmake_project_version()
{
  sed -n 's/^project(fswatch VERSION *\([^ ]*\) .*/\1/p' CMakeLists.txt |
    tail -n 1
}

release_cmake_version_modifier()
{
  sed -n 's/^set(VERSION_MODIFIER *"\([^"]*\)")$/\1/p' CMakeLists.txt |
    tail -n 1
}

release_news_top_version()
{
  (( $# == 1 )) || release_die "release_news_top_version expects one file"
  sed -n 's/^New in \(.*\):$/\1/p' "$1" |
    head -n 1
}

release_tag_exists()
{
  (( $# == 1 )) || release_die "release_tag_exists expects one tag"
  git rev-parse -q --verify "refs/tags/$1" > /dev/null
}

release_tag_type()
{
  (( $# == 1 )) || release_die "release_tag_type expects one tag"
  git cat-file -t "$1" 2> /dev/null || true
}
