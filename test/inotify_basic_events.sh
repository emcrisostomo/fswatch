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

if [ "$#" -gt 1 ]; then
  echo "usage: $0 [FSWATCH]" >&2
  exit 2
fi

FSWATCH=${1:-${FSWATCH:-}}
if [ -z "${FSWATCH}" ]; then
  echo "FSWATCH is required" >&2
  exit 2
fi

TMPDIR=${TMPDIR:-/tmp}
WORKDIR=$(mktemp -d "${TMPDIR%/}/fswatch-inotify-basic.XXXXXX")
PID=

cleanup() {
  if [ -n "${PID}" ]; then
    kill "${PID}" 2>/dev/null || true
    wait "${PID}" 2>/dev/null || true
  fi

  rm -rf "${WORKDIR}"
}

trap cleanup EXIT INT TERM

TESTDIR="${WORKDIR}/watched"
mkdir "${TESTDIR}"

"${FSWATCH}" -m inotify_monitor -r --format '%p %f' "${TESTDIR}" \
  > "${WORKDIR}/out.log" 2> "${WORKDIR}/err.log" &
PID=$!

sleep 1

CREATE_FILE="${TESTDIR}/created.txt"
MOVE_SOURCE="${TESTDIR}/move-source.txt"
MOVE_TARGET="${TESTDIR}/move-target.txt"
DELETE_FILE="${TESTDIR}/delete-me.txt"

echo created > "${CREATE_FILE}"
echo update >> "${CREATE_FILE}"
echo move > "${MOVE_SOURCE}"
mv "${MOVE_SOURCE}" "${MOVE_TARGET}"
echo delete > "${DELETE_FILE}"
rm "${DELETE_FILE}"

assert_event() {
  pattern=$1
  description=$2
  found=
  attempt=0

  while [ "${attempt}" -lt 8 ]; do
    if grep -Eq "${pattern}" "${WORKDIR}/out.log"; then
      found=1
      break
    fi

    attempt=$((attempt + 1))
    sleep 1
  done

  if [ -z "${found}" ]; then
    echo "missing inotify event: ${description}" >&2
    echo "--- fswatch output ---" >&2
    sed -n '1,160p' "${WORKDIR}/out.log" >&2
    echo "--- fswatch stderr ---" >&2
    sed -n '1,80p' "${WORKDIR}/err.log" >&2
    exit 1
  fi
}

assert_event 'created\.txt .*Created' 'file creation'
assert_event 'created\.txt .*(Updated|CloseWrite)' 'file update'
assert_event '/watched .*MovedFrom' 'rename source'
assert_event '/watched .*MovedTo' 'rename target'
assert_event 'delete-me\.txt .*Removed' 'file deletion'
