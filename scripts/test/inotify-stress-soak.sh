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

BUILD_DIR=${BUILD_DIR:-build-inotify-soak}
ITERATIONS=${ITERATIONS:-50}
JOBS=${JOBS:-2}
export CCACHE_DISABLE=${CCACHE_DISABLE:-1}

cmake -S . -B "${BUILD_DIR}" -DBUILD_TESTING=ON
cmake --build "${BUILD_DIR}" -j "${JOBS}"

i=1
while [ "${i}" -le "${ITERATIONS}" ]; do
  printf 'inotify stress soak iteration %s/%s\n' "${i}" "${ITERATIONS}"
  ctest --test-dir "${BUILD_DIR}" -L inotify --output-on-failure
  i=$((i + 1))
done
