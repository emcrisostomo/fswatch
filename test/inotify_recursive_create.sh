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

if [ "$#" -eq 2 ]; then
  FSWATCH=$1
  GIT=$2
elif [ -z "${FSWATCH:-}" ] || [ -z "${GIT:-}" ]; then
  echo "usage: $0 FSWATCH GIT" >&2
  exit 2
fi

TMPDIR=${TMPDIR:-/tmp}
WORKDIR=$(mktemp -d "${TMPDIR%/}/fswatch-inotify-recursive.XXXXXX")
PID=

cleanup() {
  if [ -n "${PID}" ]; then
    kill "${PID}" 2>/dev/null || true
    wait "${PID}" 2>/dev/null || true
  fi

  rm -rf "${WORKDIR}"
}

trap cleanup EXIT INT TERM

cd "${WORKDIR}"
"${GIT}" init -q

"${FSWATCH}" -m inotify_monitor -rx --event Created .git > "${WORKDIR}/out.log" 2> "${WORKDIR}/err.log" &
PID=$!

sleep 1

echo one > 1.txt
"${GIT}" add 1.txt

found=
attempt=0

while [ "${attempt}" -lt 8 ]; do
  if grep -Eq '/\.git/objects/[^/]+/[^/]+ Created$' "${WORKDIR}/out.log"; then
    found=1
    break
  fi

  attempt=$((attempt + 1))
  sleep 1
done

if [ -z "${found}" ]; then
  echo "missing synthetic create event for git object file" >&2
  echo "--- fswatch output ---" >&2
  sed -n '1,160p' "${WORKDIR}/out.log" >&2
  echo "--- fswatch stderr ---" >&2
  sed -n '1,80p' "${WORKDIR}/err.log" >&2
  echo "--- git object files ---" >&2
  find "${WORKDIR}/.git/objects" -type f -print >&2
  exit 1
fi
