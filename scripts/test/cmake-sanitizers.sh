#!/bin/sh
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

set -eu

BUILD_DIR=${BUILD_DIR:-build-sanitize}
JOBS=${JOBS:-2}
SANITIZERS=${SANITIZERS:-address,undefined}
SANITIZER_FLAGS="-g -O1 -fsanitize=${SANITIZERS} -fno-omit-frame-pointer"
CXX=${CXX:-c++}
export CCACHE_DISABLE=${CCACHE_DISABLE:-1}
export ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:abort_on_error=1}
export UBSAN_OPTIONS=${UBSAN_OPTIONS:-print_stacktrace=1:halt_on_error=1}

TMPDIR=${TMPDIR:-/tmp}
CHECK_DIR=$(mktemp -d "${TMPDIR%/}/fswatch-sanitizer-check.XXXXXX")
trap 'rm -rf "${CHECK_DIR}"' EXIT INT TERM

printf 'int main() { return 0; }\n' > "${CHECK_DIR}/sanitizer-check.cpp"
if ! "${CXX}" ${SANITIZER_FLAGS} "${CHECK_DIR}/sanitizer-check.cpp" \
  -o "${CHECK_DIR}/sanitizer-check" >/dev/null 2>"${CHECK_DIR}/sanitizer-check.err"; then
  echo "sanitizer compiler/link check failed for: ${SANITIZERS}" >&2
  sed -n '1,120p' "${CHECK_DIR}/sanitizer-check.err" >&2
  exit 1
fi

cmake -S . -B "${BUILD_DIR}" \
  -DBUILD_TESTING=ON \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS_DEBUG="${SANITIZER_FLAGS}" \
  -DCMAKE_CXX_FLAGS_DEBUG="${SANITIZER_FLAGS}" \
  -DCMAKE_EXE_LINKER_FLAGS_DEBUG="-fsanitize=${SANITIZERS}" \
  -DCMAKE_SHARED_LINKER_FLAGS_DEBUG="-fsanitize=${SANITIZERS}"

cmake --build "${BUILD_DIR}" -j "${JOBS}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure
