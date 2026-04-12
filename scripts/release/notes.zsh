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
  print -- "Usage: ${PROGNAME} [options] VERSION"
  print
  print -- "Generate a GitHub release-note draft from NEWS files."
  print
  print -- "Options:"
  print -- "  -o, --output FILE           Write notes to FILE instead of stdout."
  print -- "  --require-libfswatch-news  Fail if NEWS.libfswatch has no VERSION section."
  print -- "  -h, --help                 Print this message."
}

output_file=
typeset -i require_libfswatch_news=0

while (( $# > 0 ))
do
  case "$1" in
    -o|--output)
      (( $# >= 2 )) || release_die "$1 requires a file"
      output_file=$2
      shift 2
      ;;
    --require-libfswatch-news)
      require_libfswatch_news=1
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

extract_section()
{
  local file=$1
  local version=$2

  awk -v version="${version}" '
    BEGIN { found = 0; n = 0 }
    $0 == "New in " version ":" { found = 1; next }
    found && /^New in / { exit }
    found { lines[++n] = $0 }
    END {
      if (!found) exit 2
      start = 1
      while (start <= n && lines[start] == "") start++
      end = n
      while (end >= start && lines[end] == "") end--
      for (i = start; i <= end; i++) print lines[i]
    }
  ' "${file}"
}

fswatch_section=$(extract_section NEWS "${RELEASE_VERSION}") ||
  release_die "NEWS has no section for ${RELEASE_VERSION}"

libfswatch_section=
if libfswatch_section=$(extract_section NEWS.libfswatch "${RELEASE_VERSION}" 2> /dev/null)
then
  :
elif (( require_libfswatch_news ))
then
  release_die "NEWS.libfswatch has no section for ${RELEASE_VERSION}"
else
  libfswatch_section=
fi

render_notes()
{
  print -- "# What's New in fswatch v. ${RELEASE_VERSION}:"
  print
  print -- "\`fswatch\` v. ${RELEASE_VERSION} introduces the following features and bug fixes:"
  print
  print -- "${fswatch_section}"

  if [[ -n "${libfswatch_section}" ]]
  then
    print
    print -- "\`libfswatch\` v. ${RELEASE_VERSION} introduces the following features and bug fixes:"
    print
    print -- "${libfswatch_section}"
  fi
}

if [[ -n "${output_file}" ]]
then
  render_notes > "${output_file}"
  print -- "Wrote release-note draft: ${output_file}"
else
  render_notes
fi
