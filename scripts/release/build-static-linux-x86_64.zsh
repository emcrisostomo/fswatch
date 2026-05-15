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
  print -- "Usage: ${PROGNAME} VERSION"
  print
  print -- "Build fswatch-VERSION-linux-x86_64-static.tar.gz."
}

(( $# == 1 )) || {
  usage
  exit 1
}

release_parse_version "$1"

repo_root=$(release_repo_root)
cd "${repo_root}"

[[ "$(uname -s)" == "Linux" ]] ||
  release_die "static convenience binary must be built on Linux"
[[ "$(uname -m)" == "x86_64" ]] ||
  release_die "static convenience binary must be built on x86_64"

for required_command in file git ldd make readelf sha256sum tar
do
  command -v "${required_command}" > /dev/null ||
    release_die "required command not found: ${required_command}"
done

artifact="fswatch-${RELEASE_VERSION}-linux-x86_64-static.tar.gz"
work_root="${TMPDIR:-/tmp}/fswatch-static-linux-x86_64-${RELEASE_VERSION}-$$"
source_dir="${work_root}/source"
build_dir="${work_root}/build"
stage_dir="${work_root}/stage"

cleanup()
{
  rm -rf "${work_root}"
}
trap cleanup EXIT

rm -f "${artifact}"
mkdir -p "${source_dir}" "${build_dir}" "${stage_dir}"

git ls-files -z |
  tar --null -T - -cf - |
  tar -C "${source_dir}" -xf -

(
  cd "${source_dir}"
  ./autogen.sh
)

(
  cd "${build_dir}"
  "${source_dir}/configure" \
    --disable-shared \
    --enable-static \
    --disable-nls \
    CFLAGS="-O2" \
    CXXFLAGS="-O2" \
    LDFLAGS="-static"
  jobs="${JOBS:-$(getconf _NPROCESSORS_ONLN 2> /dev/null || print 2)}"
  make -j "${jobs}" -C libfswatch/src
  make -j "${jobs}" -C fswatch/src \
    AM_LDFLAGS="-all-static" \
    LDFLAGS="-static -no-pie -static-libgcc -static-libstdc++"
)

built_fswatch="${build_dir}/fswatch/src/fswatch"
if ! file "${built_fswatch}" | grep -Eq 'ELF 64-bit.*x86-64'
then
  if [[ -x "${build_dir}/fswatch/src/.libs/fswatch" ]]
  then
    built_fswatch="${build_dir}/fswatch/src/.libs/fswatch"
  fi
fi

file "${built_fswatch}" | tee "${work_root}/file.txt"
ldd "${built_fswatch}" > "${work_root}/ldd.txt" 2>&1 || true
cat "${work_root}/ldd.txt"
readelf -d "${built_fswatch}" > "${work_root}/readelf-d.txt" 2>&1 || true
cat "${work_root}/readelf-d.txt"

file "${built_fswatch}" | grep -Eq 'ELF 64-bit.*x86-64' ||
  release_die "fswatch is not a Linux x86_64 ELF executable"

if readelf -l "${built_fswatch}" | grep -q 'INTERP'
then
  release_die "fswatch has an ELF interpreter and is dynamically linked"
fi

if grep -q '(NEEDED)' "${work_root}/readelf-d.txt"
then
  release_die "fswatch has NEEDED shared library entries"
fi

if ! grep -Eiq 'not a dynamic executable|not a valid dynamic|statically linked' "${work_root}/ldd.txt"
then
  release_die "ldd did not identify fswatch as a static executable"
fi

cp "${built_fswatch}" "${stage_dir}/fswatch"
cp "${source_dir}/COPYING" "${stage_dir}/COPYING"
cat > "${stage_dir}/README.binary" <<EOF
fswatch ${RELEASE_VERSION} Linux x86_64 static convenience binary

This is an experimental convenience binary for Linux x86_64 systems only.
It is built as a static executable, using a musl-based build environment in the
GitHub release workflow.

This binary is not the recommended installation path.  Package managers and
distro packages remain the recommended way to install fswatch.  If you need
integration with your distribution, use your distribution's fswatch package
instead.

This package intentionally contains only the fswatch executable and license
metadata.  It does not include libfswatch ABI/API artifacts, headers,
pkg-config files, man pages, shared libraries, or static libraries.
EOF

(
  cd "${stage_dir}"
  sha256sum fswatch COPYING README.binary > SHA256SUMS
  tar -czf "${repo_root}/${artifact}" fswatch COPYING README.binary SHA256SUMS
)

tar -tzf "${artifact}" | sed '/./!d' > "${work_root}/tar-contents.txt"
if ! cmp -s "${work_root}/tar-contents.txt" =(print -l fswatch COPYING README.binary SHA256SUMS)
then
  print -u2 -- "unexpected tarball contents:"
  cat "${work_root}/tar-contents.txt" >&2
  release_die "${artifact} does not contain exactly the expected files"
fi

release_info "created ${artifact}"
