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
  print -- "Usage: ${PROGNAME} [options] [VERSION]"
  print
  print -- "Watch the GitHub release workflow and verify uploaded release assets."
  print -- "When VERSION is omitted, read it from m4/fswatch_version.m4."
  print
  print -- "Options:"
  print -- "  --repo OWNER/REPO   GitHub repository. Default: ${RELEASE_REPO}."
  print -- "  --workflow NAME     Workflow file or name. Default: release-tarball.yml."
  print -- "  --run-id ID         Use an explicit GitHub Actions run ID."
  print -- "  --no-watch          Do not wait; only inspect the current run state."
  print -- "  --timeout SECONDS   Wait up to this long for the run to appear. Default: 120."
  print -- "  --poll SECONDS      Poll interval while waiting for the run. Default: 5."
  print -- "  -h, --help          Print this message."
}

repo=${RELEASE_REPO}
workflow=release-tarball.yml
run_id=
typeset -i do_watch=1
typeset -i timeout_seconds=120
typeset -i poll_seconds=5

while (( $# > 0 ))
do
  case "$1" in
    --repo)
      (( $# >= 2 )) || release_die "--repo requires OWNER/REPO"
      repo=$2
      shift 2
      ;;
    --workflow)
      (( $# >= 2 )) || release_die "--workflow requires a workflow file or name"
      workflow=$2
      shift 2
      ;;
    --run-id)
      (( $# >= 2 )) || release_die "--run-id requires a GitHub Actions run ID"
      run_id=$2
      shift 2
      ;;
    --no-watch)
      do_watch=0
      shift
      ;;
    --timeout)
      (( $# >= 2 )) || release_die "--timeout requires seconds"
      [[ "$2" == <-> ]] || release_die "--timeout requires a non-negative integer"
      timeout_seconds=$2
      shift 2
      ;;
    --poll)
      (( $# >= 2 )) || release_die "--poll requires seconds"
      [[ "$2" == <-> ]] || release_die "--poll requires a non-negative integer"
      (( $2 > 0 )) || release_die "--poll must be greater than zero"
      poll_seconds=$2
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

(( $# <= 1 )) || {
  usage
  exit 1
}

if (( $# == 1 ))
then
  release_version=$1
else
  repo_root=$(release_repo_root)
  cd "${repo_root}"
  release_version=$(release_m4_define m4/fswatch_version.m4 FSWATCH_VERSION)
  [[ -n "${release_version}" ]] ||
    release_die "could not read FSWATCH_VERSION from m4/fswatch_version.m4"
fi

release_parse_version "${release_version}"
(( ! RELEASE_IS_DEVELOP )) ||
  release_die "release workflow checks require a releasable version, not -develop"

command -v gh > /dev/null ||
  release_die "gh is required to inspect GitHub Actions and releases"

if [[ -z "${run_id}" ]]
then
  typeset -i waited_seconds=0
  while true
  do
    run_id=$(gh run list \
      --repo "${repo}" \
      --workflow "${workflow}" \
      --event release \
      --limit 20 \
      --json databaseId,headBranch \
      --jq ".[] | select(.headBranch == \"${RELEASE_VERSION}\") | .databaseId" |
      sed -n '1p')

    [[ -n "${run_id}" ]] && break

    (( waited_seconds >= timeout_seconds )) && break

    print -- "Waiting for ${workflow} release run for ${RELEASE_VERSION}..."
    sleep "${poll_seconds}"
    waited_seconds+=poll_seconds
  done

  [[ -n "${run_id}" ]] || {
    gh run list \
      --repo "${repo}" \
      --workflow "${workflow}" \
      --event release \
      --limit 10
    release_die "could not find a ${workflow} release run for ${RELEASE_VERSION}"
  }
fi

print -- "Release workflow run for ${RELEASE_VERSION}: ${run_id}"

if (( do_watch ))
then
  gh run watch "${run_id}" --repo "${repo}" --exit-status
fi

run_status=$(gh run view "${run_id}" --repo "${repo}" --json status --jq .status)
run_conclusion=$(gh run view "${run_id}" --repo "${repo}" --json conclusion --jq .conclusion)
run_url=$(gh run view "${run_id}" --repo "${repo}" --json url --jq .url)

print -- "Workflow status: ${run_status}"
print -- "Workflow conclusion: ${run_conclusion}"
print -- "Workflow URL: ${run_url}"

[[ "${run_status}" == "completed" ]] ||
  release_die "workflow run has not completed"

[[ "${run_conclusion}" == "success" ]] ||
  release_die "workflow run did not succeed"

release_url=$(gh release view "${RELEASE_VERSION}" --repo "${repo}" --json url --jq .url)
release_tag=$(gh release view "${RELEASE_VERSION}" --repo "${repo}" --json tagName --jq .tagName)
asset_names=$(gh release view "${RELEASE_VERSION}" --repo "${repo}" --json assets --jq '.assets[].name')

[[ "${release_tag}" == "${RELEASE_VERSION}" ]] ||
  release_die "GitHub release tag is ${release_tag}, expected ${RELEASE_VERSION}"

expected_tarball="fswatch-${RELEASE_VERSION}.tar.gz"
expected_pdf="fswatch-${RELEASE_VERSION}.pdf"
typeset -i failures=0

if print -r -- "${asset_names}" | grep -Fxq "${expected_tarball}"
then
  print -- "ok: release asset exists: ${expected_tarball}"
else
  print -u2 -- "FAIL: release asset is missing: ${expected_tarball}"
  failures+=1
fi

if print -r -- "${asset_names}" | grep -Fxq "${expected_pdf}"
then
  print -- "ok: release asset exists: ${expected_pdf}"
else
  print -u2 -- "FAIL: release asset is missing: ${expected_pdf}"
  failures+=1
fi

extra_assets=$(print -r -- "${asset_names}" |
  grep -Fvx -e "${expected_tarball}" -e "${expected_pdf}" || true)

if [[ -n "${extra_assets}" ]]
then
  release_warn "unexpected explicit release assets:"
  print -u2 -- "${extra_assets}"
fi

(( failures == 0 )) ||
  release_die "release asset verification failed"

print -- "GitHub release URL: ${release_url}"
print -- "Release workflow and assets verified for ${RELEASE_VERSION}."
