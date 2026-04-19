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

PROGNAME=${0:t}
PROGDIR=${0:A:h}

. "${PROGDIR}/lib.zsh"

usage()
{
  print -- "Usage: ${PROGNAME} [--apply] VERSION"
  print
  print -- "Prepare version files for VERSION."
  print -- "Without --apply, only print the edits that would be made."
  print
  print -- "VERSION may be X.Y.Z, X.Y.Z-rcN, or X.Y.Z-develop."
}

typeset -i apply=0

while (( $# > 0 ))
do
  case "$1" in
    --apply)
      apply=1
      shift
      ;;
    --dry-run)
      apply=0
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      break
      ;;
    -*)
      release_die "unknown option: $1"
      ;;
    *)
      break
      ;;
  esac
done

(( $# == 1 )) || {
  usage
  exit 1
}

release_parse_version "$1"

repo_root=$(release_repo_root)
cd "${repo_root}"

print -- "Version file preparation:"
print -- "  FSWATCH_VERSION=${RELEASE_VERSION}"
print -- "  LIBFSWATCH_VERSION=${RELEASE_VERSION}"
print -- "  CMake project version=${RELEASE_BASE_VERSION}"
print -- "  CMake VERSION_MODIFIER=${RELEASE_VERSION_MODIFIER}"

if (( ! apply ))
then
  print -- "Dry run only. Re-run with --apply to edit files."
  exit 0
fi

tmp_file=m4/fswatch_version.m4.tmp.$$
awk -v value="${RELEASE_VERSION}" '
  /^m4_define\(\[FSWATCH_VERSION\], \[/ {
    print "m4_define([FSWATCH_VERSION], [" value "])"
    next
  }
  { print }
' m4/fswatch_version.m4 > "${tmp_file}"
mv "${tmp_file}" m4/fswatch_version.m4

tmp_file=m4/libfswatch_version.m4.tmp.$$
awk -v value="${RELEASE_VERSION}" '
  /^m4_define\(\[LIBFSWATCH_VERSION\], \[/ {
    print "m4_define([LIBFSWATCH_VERSION], [" value "])"
    next
  }
  { print }
' m4/libfswatch_version.m4 > "${tmp_file}"
mv "${tmp_file}" m4/libfswatch_version.m4

tmp_file=CMakeLists.txt.tmp.$$
awk -v base="${RELEASE_BASE_VERSION}" -v modifier="${RELEASE_VERSION_MODIFIER}" '
  /^project\(fswatch VERSION / {
    print "project(fswatch VERSION " base " LANGUAGES C CXX)"
    next
  }
  /^set\(VERSION_MODIFIER / {
    print "set(VERSION_MODIFIER \"" modifier "\")"
    next
  }
  { print }
' CMakeLists.txt > "${tmp_file}"
mv "${tmp_file}" CMakeLists.txt

[[ "$(release_m4_define m4/fswatch_version.m4 FSWATCH_VERSION)" == "${RELEASE_VERSION}" ]] ||
  release_die "failed to update FSWATCH_VERSION"
[[ "$(release_m4_define m4/libfswatch_version.m4 LIBFSWATCH_VERSION)" == "${RELEASE_VERSION}" ]] ||
  release_die "failed to update LIBFSWATCH_VERSION"
[[ "$(release_cmake_project_version)" == "${RELEASE_BASE_VERSION}" ]] ||
  release_die "failed to update CMake project version"
[[ "$(release_cmake_version_modifier)" == "${RELEASE_VERSION_MODIFIER}" ]] ||
  release_die "failed to update CMake VERSION_MODIFIER"

print -- "Updated version files. Review LIBFSWATCH_API_VERSION and changelogs manually."
print -- "When a changelog item comes from a GitHub PR or issue, include those numbers."
