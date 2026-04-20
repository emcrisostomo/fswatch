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

if [ "$#" -eq 1 ]; then
  FSWATCH=$1
elif [ -z "${FSWATCH:-}" ]; then
  echo "usage: $0 FSWATCH" >&2
  exit 2
fi

if ! "${FSWATCH}" -M | grep -q '^  fanotify_monitor$'; then
  echo "fanotify monitor is not built on this platform" >&2
  exit 77
fi

TMPDIR=${TMPDIR:-/tmp}
WORKDIR=$(mktemp -d "${TMPDIR%/}/fswatch-fanotify-basic.XXXXXX")
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

"${FSWATCH}" -m fanotify_monitor -r --format '%p %f %K:%P' "${TESTDIR}" \
  > "${WORKDIR}/out.log" 2> "${WORKDIR}/err.log" &
PID=$!

sleep 1

if ! kill -0 "${PID}" 2>/dev/null; then
  if grep -Eqi 'fanotify|permission|operation not permitted|not supported|Cannot initialize' "${WORKDIR}/err.log"; then
    echo "fanotify is unavailable in this environment" >&2
    sed -n '1,120p' "${WORKDIR}/err.log" >&2
    exit 77
  fi

  echo "fanotify monitor exited unexpectedly" >&2
  sed -n '1,120p' "${WORKDIR}/err.log" >&2
  exit 1
fi

echo one > "${TESTFILE}"
echo two >> "${TESTFILE}"
rm "${TESTFILE}"

found_create=
found_update=
found_remove=
attempt=0

while [ "${attempt}" -lt 10 ]; do
  if grep -q ' Created' "${WORKDIR}/out.log"; then found_create=1; fi
  if grep -Eq ' (Updated|CloseWrite)' "${WORKDIR}/out.log"; then found_update=1; fi
  if grep -q ' Removed' "${WORKDIR}/out.log"; then found_remove=1; fi

  if [ -n "${found_create}" ] && [ -n "${found_update}" ] && [ -n "${found_remove}" ]; then
    break
  fi

  attempt=$((attempt + 1))
  sleep 1
done

if [ -z "${found_create}" ] || [ -z "${found_update}" ] || [ -z "${found_remove}" ]; then
  echo "missing expected fanotify events" >&2
  echo "--- fswatch output ---" >&2
  sed -n '1,160p' "${WORKDIR}/out.log" >&2
  echo "--- fswatch stderr ---" >&2
  sed -n '1,160p' "${WORKDIR}/err.log" >&2
  exit 1
fi
