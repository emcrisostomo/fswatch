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
  print -- "Usage: ${PROGNAME} [--apply] NEXT_VERSION"
  print
  print -- "Prepare version files for the next development cycle."
  print -- "NEXT_VERSION must be numeric X.Y.Z; this script appends -develop."
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

next_version=$1
release_parse_version "${next_version}"

[[ -z "${RELEASE_VERSION_MODIFIER}" ]] ||
  release_die "NEXT_VERSION must be numeric X.Y.Z; got ${next_version}"

develop_version="${next_version}-develop"

if (( apply ))
then
  "${PROGDIR}/prepare.zsh" --apply "${develop_version}"
  current_news_version=$(release_news_top_version NEWS)
  if [[ "${current_news_version}" != "${develop_version}" ]]
  then
    tmp_file=NEWS.tmp.$$
    awk -v version="${develop_version}" '
      NR == 1 { print; next }
      NR == 2 {
        print
        print ""
        print "New in " version ":"
        print ""
        next
      }
      { print }
    ' NEWS > "${tmp_file}"
    mv "${tmp_file}" NEWS
    print -- "Added NEWS section for ${develop_version}."
  fi
  print -- "Review the diff, then commit and push the post-release development bump."
else
  print -- "Dry run only. These commands would prepare the next development cycle:"
  print -- "  scripts/release/prepare.zsh --apply '${develop_version}'"
  print -- "  ensure NEWS starts with 'New in ${develop_version}:'"
  print -- "  git add CMakeLists.txt m4/fswatch_version.m4 m4/libfswatch_version.m4 NEWS"
  print -- "  git commit -m 'Bump version to ${develop_version}'"
  print -- "  git push origin master"
fi
