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
  print -- "Usage: ${PROGNAME} [options]"
  print
  print -- "Run non-publishing tests for the release automation."
  print
  print -- "Options:"
  print -- "  --release VERSION       Simulated release version. Default: 1.19.0."
  print -- "  --next VERSION          Simulated next development version. Default: 1.20.0."
  print -- "  --dist                  Also run autogen, configure, and make -j dist."
  print -- "  --distcheck             Also run autogen, configure, and make -j distcheck."
  print -- "  --keep-worktree         Keep the temporary worktree for inspection."
  print -- "  -h, --help              Print this message."
}

release_version=1.19.0
next_version=1.20.0
typeset -i run_dist=0
typeset -i run_distcheck=0
typeset -i keep_worktree=0

while (( $# > 0 ))
do
  case "$1" in
    --release)
      (( $# >= 2 )) || release_die "--release requires VERSION"
      release_version=$2
      shift 2
      ;;
    --next)
      (( $# >= 2 )) || release_die "--next requires VERSION"
      next_version=$2
      shift 2
      ;;
    --dist)
      run_dist=1
      shift
      ;;
    --distcheck)
      run_distcheck=1
      shift
      ;;
    --keep-worktree)
      keep_worktree=1
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
      release_die "unexpected argument: $1"
      ;;
  esac
done

release_parse_version "${release_version}"
(( ! RELEASE_IS_DEVELOP )) ||
  release_die "--release must be a releasable version, not -develop"

release_parse_version "${next_version}"
[[ -z "${RELEASE_VERSION_MODIFIER}" ]] ||
  release_die "--next must be numeric X.Y.Z"

repo_root=$(release_repo_root)
cd "${repo_root}"

tmp_parent=$(mktemp -d /tmp/fswatch-release-test.XXXXXX)
tmp_worktree="${tmp_parent}/worktree"

cleanup()
{
  local exit_status=$?

  if (( keep_worktree ))
  then
    print -- "Kept temporary worktree: ${tmp_worktree}"
    print -- "Remove it with: git worktree remove '${tmp_worktree}'"
  else
    git worktree remove --force "${tmp_worktree}" > /dev/null 2>&1 || true
    rm -rf "${tmp_parent}"
  fi

  exit ${exit_status}
}

trap cleanup EXIT INT TERM

print -- "Creating temporary worktree: ${tmp_worktree}"
git worktree add --detach --quiet "${tmp_worktree}" HEAD

print -- "Overlaying current release scripts into temporary worktree."
rm -rf "${tmp_worktree}/scripts/release"
mkdir -p "${tmp_worktree}/scripts"
cp -R "${repo_root}/scripts/release" "${tmp_worktree}/scripts/"

cd "${tmp_worktree}"

print -- "Running shell syntax checks."
zsh -n scripts/release/*.zsh

print -- "Checking current development metadata."
current_version=$(release_m4_define m4/fswatch_version.m4 FSWATCH_VERSION)
scripts/release/check.zsh "${current_version}"

release_parse_version "${current_version}"
current_is_develop=${RELEASE_IS_DEVELOP}

print -- "Checking release guards."
if (( current_is_develop ))
then
  if scripts/release/check.zsh --release "${current_version}" > release-check-develop.log 2>&1
  then
    release_die "--release unexpectedly accepted ${current_version}"
  fi
else
  scripts/release/check.zsh --release "${current_version}" > /dev/null
fi

if [[ "${current_version}" != "${release_version}" ]]
then
  if scripts/release/check.zsh --release "${release_version}" > release-check-unprepared.log 2>&1
  then
    release_die "unprepared release check unexpectedly passed for ${release_version}"
  fi
fi

print -- "Running dry-run helpers."
scripts/release/prepare.zsh "${release_version}" > /dev/null
scripts/release/post-release.zsh "${next_version}" > /dev/null

print -- "Preparing simulated release ${release_version}."
scripts/release/prepare.zsh --apply "${release_version}"

tmp_news=NEWS.tmp.$$
awk -v version="${release_version}" -v old_version="${current_version}" '
  NR == 1 { print; next }
  NR == 2 {
    print
    print ""
    print "New in " version ":"
    print ""
    next
  }
  $0 == "New in " old_version ":" { next }
  { print }
' NEWS > "${tmp_news}"
mv "${tmp_news}" NEWS

scripts/release/check.zsh "${release_version}"

print -- "Generating and validating release notes."
scripts/release/notes.zsh --output release-notes.md "${release_version}"
scripts/release/check.zsh --release --notes release-notes.md "${release_version}"

print -- "Checking tarball-name validation with a dummy file."
touch "fswatch-${release_version}.tar.gz"
scripts/release/check.zsh --release --require-dist "${release_version}"
rm "fswatch-${release_version}.tar.gz"

print -- "Checking publish dry-run."
scripts/release/publish.zsh --notes release-notes.md "${release_version}" > publish-dry-run.log

if (( run_dist || run_distcheck ))
then
  print -- "Bootstrapping and configuring the temporary release worktree."
  ./autogen.sh
  CC=${CC:-gcc} CXX=${CXX:-g++} ./configure
fi

if (( run_dist ))
then
  print -- "Running make -j dist."
  make -j dist
  scripts/release/check.zsh --release --require-dist "${release_version}"
fi

if (( run_distcheck ))
then
  print -- "Running make -j distcheck."
  make -j distcheck
  scripts/release/check.zsh --release --require-dist "${release_version}"
fi

print -- "Preparing simulated post-release development version ${next_version}-develop."
scripts/release/post-release.zsh --apply "${next_version}"
scripts/release/check.zsh "${next_version}-develop"

print -- "Release automation tests passed."
