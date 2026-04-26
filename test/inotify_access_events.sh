#!/bin/sh
#
# Copyright (c) 2014-2026 Enrico M. Crisostomo
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
WORKDIR=$(mktemp -d "${TMPDIR%/}/fswatch-inotify-access.XXXXXX")
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
TESTFILE="${TESTDIR}/file.txt"
mkdir "${TESTDIR}"
echo "content" > "${TESTFILE}"

start_fswatch() {
  access_flag=$1

  : > "${WORKDIR}/out.log"
  : > "${WORKDIR}/err.log"

  if [ -n "${access_flag}" ]; then
    "${FSWATCH}" -m inotify_monitor -rx "${access_flag}" --event PlatformSpecific "${TESTDIR}" \
      > "${WORKDIR}/out.log" 2> "${WORKDIR}/err.log" &
  else
    "${FSWATCH}" -m inotify_monitor -rx --event PlatformSpecific "${TESTDIR}" \
      > "${WORKDIR}/out.log" 2> "${WORKDIR}/err.log" &
  fi
  PID=$!

  sleep 1
}

stop_fswatch() {
  if [ -n "${PID}" ]; then
    kill "${PID}" 2>/dev/null || true
    wait "${PID}" 2>/dev/null || true
    PID=
  fi
}

print_logs() {
  echo "--- fswatch output ---" >&2
  sed -n '1,160p' "${WORKDIR}/out.log" >&2
  echo "--- fswatch stderr ---" >&2
  sed -n '1,80p' "${WORKDIR}/err.log" >&2
}

start_fswatch ""
ls "${TESTDIR}" >/dev/null
cat "${TESTFILE}" >/dev/null
sleep 1
stop_fswatch

if [ -s "${WORKDIR}/out.log" ]; then
  echo "read-only operations produced access events without --access" >&2
  print_logs
  exit 1
fi

start_fswatch "-a"
found=
attempt=0

while [ "${attempt}" -lt 8 ]; do
  cat "${TESTFILE}" >/dev/null

  if grep -q 'PlatformSpecific' "${WORKDIR}/out.log"; then
    found=1
    break
  fi

  attempt=$((attempt + 1))
  sleep 1
done

stop_fswatch

if [ -z "${found}" ]; then
  echo "missing access event when --access is specified" >&2
  print_logs
  exit 1
fi
