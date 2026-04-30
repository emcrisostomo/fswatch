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

if [ "$#" -gt 2 ]; then
  echo "usage: $0 [FSWATCH] [MONITOR]" >&2
  exit 2
fi

FSWATCH=${1:-${FSWATCH:-}}
MONITOR=${2:-${MONITOR:-}}
if [ -z "${FSWATCH}" ]; then
  echo "FSWATCH is required" >&2
  exit 2
fi
if [ -z "${MONITOR}" ]; then
  echo "MONITOR is required" >&2
  exit 2
fi
if ! "${FSWATCH}" -M | grep -q "^  ${MONITOR}$"; then
  echo "${MONITOR} is not built on this platform" >&2
  exit 77
fi

TMPDIR=${TMPDIR:-/tmp}
WORKDIR=$(mktemp -d "${TMPDIR%/}/fswatch-prune.XXXXXX")
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
PRUNED="${TESTDIR}/pruned"
VISIBLE="${TESTDIR}/visible"

mkdir -p "${PRUNED}" "${VISIBLE}"

"${FSWATCH}" -m "${MONITOR}" -r --format '%p %f' --event Created \
  --prune '/pruned$' "${TESTDIR}" \
  > "${WORKDIR}/out.log" 2> "${WORKDIR}/err.log" &
PID=$!

sleep 1

if ! kill -0 "${PID}" 2>/dev/null; then
  if [ "${MONITOR}" = "fanotify_monitor" ] &&
     grep -Eqi 'fanotify|permission|operation not permitted|not supported|Cannot initialize' "${WORKDIR}/err.log"; then
    echo "fanotify is unavailable in this environment" >&2
    sed -n '1,120p' "${WORKDIR}/err.log" >&2
    exit 77
  fi

  echo "${MONITOR} exited unexpectedly" >&2
  sed -n '1,120p' "${WORKDIR}/err.log" >&2
  exit 1
fi

echo visible > "${VISIBLE}/visible.txt"
echo hidden > "${PRUNED}/hidden.txt"

found=
attempt=0
while [ "${attempt}" -lt 10 ]; do
  if grep -Eq '/visible/visible\.txt .*Created' "${WORKDIR}/out.log"; then
    found=1
    break
  fi

  attempt=$((attempt + 1))
  sleep 1
done

if [ -z "${found}" ]; then
  echo "missing event outside pruned directory" >&2
  echo "--- fswatch output ---" >&2
  sed -n '1,160p' "${WORKDIR}/out.log" >&2
  echo "--- fswatch stderr ---" >&2
  sed -n '1,80p' "${WORKDIR}/err.log" >&2
  exit 1
fi

sleep 1

if grep -Eq '/pruned/hidden\.txt' "${WORKDIR}/out.log"; then
  echo "event below pruned directory was reported" >&2
  echo "--- fswatch output ---" >&2
  sed -n '1,160p' "${WORKDIR}/out.log" >&2
  echo "--- fswatch stderr ---" >&2
  sed -n '1,80p' "${WORKDIR}/err.log" >&2
  exit 1
fi
