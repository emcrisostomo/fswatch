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
  print -- "Validate fswatch release metadata without modifying the repository."
  print
  print -- "Options:"
  print -- "  --release                  Require VERSION to be a releasable version, not -develop."
  print -- "  --require-clean            Require no tracked working-tree changes."
  print -- "  --require-dist             Require fswatch-VERSION.tar.gz to exist."
  print -- "  --require-annotated-tag    Require VERSION to exist as an annotated local tag."
  print -- "  --require-libfswatch-news  Require NEWS.libfswatch top section to match VERSION."
  print -- "  --notes FILE               Validate a GitHub release notes file."
  print -- "  -h, --help                 Print this message."
}

typeset -i release_mode=0
typeset -i require_clean=0
typeset -i require_dist=0
typeset -i require_annotated_tag=0
typeset -i require_libfswatch_news=0
notes_file=

while (( $# > 0 ))
do
  case "$1" in
    --release)
      release_mode=1
      shift
      ;;
    --require-clean)
      require_clean=1
      shift
      ;;
    --require-dist)
      require_dist=1
      shift
      ;;
    --require-annotated-tag)
      require_annotated_tag=1
      shift
      ;;
    --require-libfswatch-news)
      require_libfswatch_news=1
      shift
      ;;
    --notes)
      (( $# >= 2 )) || release_die "--notes requires a file"
      notes_file=$2
      shift 2
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

typeset -i failures=0

fail()
{
  print -u2 -- "FAIL: $*"
  failures+=1
}

pass()
{
  print -- "ok: $*"
}

check_equal()
{
  local label=$1
  local expected=$2
  local actual=$3

  if [[ "${actual}" == "${expected}" ]]
  then
    pass "${label} is ${expected}"
  else
    fail "${label}: expected '${expected}', found '${actual}'"
  fi
}

if (( release_mode && RELEASE_IS_DEVELOP ))
then
  fail "--release does not allow -develop versions"
fi

if (( require_clean ))
then
  tracked_status=$(git status --short --untracked-files=no)
  if [[ -z "${tracked_status}" ]]
  then
    pass "tracked working tree is clean"
  else
    fail "tracked working tree has changes"
    print -u2 -- "${tracked_status}"
  fi
fi

fswatch_version=$(release_m4_define m4/fswatch_version.m4 FSWATCH_VERSION)
libfswatch_version=$(release_m4_define m4/libfswatch_version.m4 LIBFSWATCH_VERSION)
libfswatch_api_version=$(release_m4_define m4/libfswatch_version.m4 LIBFSWATCH_API_VERSION)
cmake_project_version=$(release_cmake_project_version)
cmake_version_modifier=$(release_cmake_version_modifier)
news_version=$(release_news_top_version NEWS)
libfswatch_news_version=$(release_news_top_version NEWS.libfswatch)

check_equal "FSWATCH_VERSION" "${RELEASE_VERSION}" "${fswatch_version}"
check_equal "LIBFSWATCH_VERSION" "${RELEASE_VERSION}" "${libfswatch_version}"
check_equal "CMake project version" "${RELEASE_BASE_VERSION}" "${cmake_project_version}"
check_equal "CMake VERSION_MODIFIER" "${RELEASE_VERSION_MODIFIER}" "${cmake_version_modifier}"
check_equal "NEWS top version" "${RELEASE_VERSION}" "${news_version}"

if [[ "${libfswatch_news_version}" == "${RELEASE_VERSION}" ]]
then
  pass "NEWS.libfswatch top version is ${RELEASE_VERSION}"
elif (( require_libfswatch_news ))
then
  fail "NEWS.libfswatch: expected top version '${RELEASE_VERSION}', found '${libfswatch_news_version}'"
else
  release_warn "NEWS.libfswatch top version is '${libfswatch_news_version}', not '${RELEASE_VERSION}'"
fi

if [[ "${libfswatch_api_version}" == <->:<->:<-> ]]
then
  pass "LIBFSWATCH_API_VERSION is ${libfswatch_api_version}"
else
  fail "LIBFSWATCH_API_VERSION is not current:revision:age: '${libfswatch_api_version}'"
fi

if (( require_dist ))
then
  dist_file="fswatch-${RELEASE_VERSION}.tar.gz"
  if [[ -f "${dist_file}" ]]
  then
    pass "distribution tarball exists: ${dist_file}"
  else
    fail "distribution tarball is missing: ${dist_file}"
  fi
fi

if (( require_annotated_tag ))
then
  tag_type=$(release_tag_type "${RELEASE_VERSION}")
  if [[ "${tag_type}" == "tag" ]]
  then
    pass "local tag ${RELEASE_VERSION} is annotated"
  elif [[ -z "${tag_type}" ]]
  then
    fail "local tag ${RELEASE_VERSION} does not exist"
  else
    fail "local tag ${RELEASE_VERSION} is '${tag_type}', not an annotated tag"
  fi
fi

if [[ -n "${notes_file}" ]]
then
  if [[ ! -f "${notes_file}" ]]
  then
    fail "release notes file does not exist: ${notes_file}"
  else
    expected_title="# What's New in fswatch v. ${RELEASE_VERSION}:"
    if grep -Fqx "${expected_title}" "${notes_file}"
    then
      pass "release notes title matches ${RELEASE_VERSION}"
    else
      fail "release notes title is missing: ${expected_title}"
    fi

    expected_intro="\`fswatch\` v. ${RELEASE_VERSION} introduces the following features and bug fixes:"
    if grep -Fqx "${expected_intro}" "${notes_file}"
    then
      pass "release notes fswatch intro matches ${RELEASE_VERSION}"
    else
      fail "release notes fswatch intro is missing"
    fi
  fi
fi

if (( failures > 0 ))
then
  print -u2 -- "${failures} release check(s) failed."
  exit 1
fi

print -- "All release checks passed for ${RELEASE_VERSION}."
