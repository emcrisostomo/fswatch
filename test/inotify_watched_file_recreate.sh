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
WORKDIR=$(mktemp -d "${TMPDIR%/}/fswatch-inotify-recreate.XXXXXX")
PID=

cleanup() {
  if [ -n "${PID}" ]; then
    kill "${PID}" 2>/dev/null || true
    wait "${PID}" 2>/dev/null || true
  fi

  rm -rf "${WORKDIR}"
}

trap cleanup EXIT INT TERM

WATCHED="${WORKDIR}/watched-file.txt"
echo initial > "${WATCHED}"

"${FSWATCH}" -m inotify_monitor -1 -x -l 1 \
  --event Updated --event CloseWrite "${WATCHED}" \
  > "${WORKDIR}/out.log" 2> "${WORKDIR}/err.log" &
PID=$!

sleep 1

rm "${WATCHED}"
sleep 1
echo recreated > "${WATCHED}"

found=
attempt=0

while [ "${attempt}" -lt 8 ]; do
  echo update >> "${WATCHED}"

  if grep -Eq 'watched-file\.txt .*(Updated|CloseWrite)' "${WORKDIR}/out.log"; then
    found=1
    break
  fi

  if ! kill -0 "${PID}" 2>/dev/null; then
    break
  fi

  attempt=$((attempt + 1))
  sleep 1
done

if [ -z "${found}" ]; then
  echo "missing event after watched file was deleted and recreated" >&2
  echo "--- fswatch output ---" >&2
  sed -n '1,160p' "${WORKDIR}/out.log" >&2
  echo "--- fswatch stderr ---" >&2
  sed -n '1,120p' "${WORKDIR}/err.log" >&2
  exit 1
fi
