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
  print -- "Create the release tag and GitHub release. Defaults to dry-run."
  print
  print -- "Options:"
  print -- "  --execute        Actually create/push the tag and create the GitHub release."
  print -- "  --notes FILE     Release notes file. Default: release-notes.md."
  print -- "  --repo OWNER/REPO GitHub repository. Default: ${RELEASE_REPO}."
  print -- "  -h, --help       Print this message."
}

typeset -i execute=0
notes_file=release-notes.md
repo=${RELEASE_REPO}

while (( $# > 0 ))
do
  case "$1" in
    --execute)
      execute=1
      shift
      ;;
    --dry-run)
      execute=0
      shift
      ;;
    --notes)
      (( $# >= 2 )) || release_die "--notes requires a file"
      notes_file=$2
      shift 2
      ;;
    --repo)
      (( $# >= 2 )) || release_die "--repo requires OWNER/REPO"
      repo=$2
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
(( ! RELEASE_IS_DEVELOP )) ||
  release_die "publish refuses -develop versions"

repo_root=$(release_repo_root)
cd "${repo_root}"

[[ -f "${notes_file}" ]] ||
  release_die "release notes file does not exist: ${notes_file}"

release_flags=()
if (( RELEASE_IS_PRERELEASE ))
then
  release_flags+=( --prerelease --latest=false )
fi

print -- "Publish plan for ${RELEASE_VERSION}:"
print -- "  repository: ${repo}"
print -- "  notes: ${notes_file}"
print -- "  prerelease: ${RELEASE_IS_PRERELEASE}"
print -- "  assets: produced later by .github/workflows/release-tarball.yml"

if (( ! execute ))
then
  print
  print -- "Dry run only. These commands would run:"
  print -- "  scripts/release/check.zsh --release --require-clean --notes '${notes_file}' '${RELEASE_VERSION}'"
  print -- "  git tag -a '${RELEASE_VERSION}' -m '${RELEASE_VERSION}'   # only if the tag does not exist"
  print -- "  git push origin '${RELEASE_VERSION}'"
  print -- "  gh release create '${RELEASE_VERSION}' --repo '${repo}' --verify-tag --title 'fswatch v. ${RELEASE_VERSION}' --notes-file '${notes_file}' ${release_flags[*]}"
  exit 0
fi

"${PROGDIR}/check.zsh" --release --require-clean --notes "${notes_file}" "${RELEASE_VERSION}"

if release_tag_exists "${RELEASE_VERSION}"
then
  tag_type=$(release_tag_type "${RELEASE_VERSION}")
  [[ "${tag_type}" == "tag" ]] ||
    release_die "local tag ${RELEASE_VERSION} exists but is '${tag_type}', not an annotated tag"
else
  git tag -a "${RELEASE_VERSION}" -m "${RELEASE_VERSION}"
fi

git push origin "${RELEASE_VERSION}"

gh release create "${RELEASE_VERSION}" \
  --repo "${repo}" \
  --verify-tag \
  --title "fswatch v. ${RELEASE_VERSION}" \
  --notes-file "${notes_file}" \
  "${release_flags[@]}"
