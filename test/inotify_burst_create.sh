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
WORKDIR=$(mktemp -d "${TMPDIR%/}/fswatch-inotify-burst.XXXXXX")
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

"${FSWATCH}" -m inotify_monitor -r --format '%p %f' --event Created "${TESTDIR}" \
  > "${WORKDIR}/out.log" 2> "${WORKDIR}/err.log" &
PID=$!

sleep 1

expected=250
i=0
while [ "${i}" -lt "${expected}" ]; do
  printf '%s\n' "${i}" > "${TESTDIR}/burst-${i}.txt"
  i=$((i + 1))
done

found=
attempt=0

while [ "${attempt}" -lt 10 ]; do
  count=$(grep -Ec '/burst-[0-9]+\.txt Created$' "${WORKDIR}/out.log" || true)
  if [ "${count}" -ge "${expected}" ]; then
    found=1
    break
  fi

  attempt=$((attempt + 1))
  sleep 1
done

if [ -z "${found}" ]; then
  echo "missing burst create events: expected ${expected}, found ${count:-0}" >&2
  echo "--- fswatch output ---" >&2
  sed -n '1,220p' "${WORKDIR}/out.log" >&2
  echo "--- fswatch stderr ---" >&2
  sed -n '1,120p' "${WORKDIR}/err.log" >&2
  exit 1
fi
