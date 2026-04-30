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

check_fsevents_available() {
  CHECKDIR="${WORKDIR}/fsevents-check"
  mkdir "${CHECKDIR}"

  "${FSWATCH}" -m "${MONITOR}" -r --format '%p %f' --event Created \
    "${CHECKDIR}" > "${WORKDIR}/fsevents-check.out" 2> "${WORKDIR}/fsevents-check.err" &
  CHECK_PID=$!

  sleep 1
  echo check > "${CHECKDIR}/check.txt"

  found=
  attempt=0
  while [ "${attempt}" -lt 10 ]; do
    if grep -Eq '/check\.txt .*Created' "${WORKDIR}/fsevents-check.out"; then
      found=1
      break
    fi

    if ! kill -0 "${CHECK_PID}" 2>/dev/null; then
      break
    fi

    attempt=$((attempt + 1))
    sleep 1
  done

  kill "${CHECK_PID}" 2>/dev/null || true
  wait "${CHECK_PID}" 2>/dev/null || true
  CHECK_PID=

  if [ -z "${found}" ]; then
    echo "fsevents did not deliver a basic create event in this environment" >&2
    sed -n '1,80p' "${WORKDIR}/fsevents-check.err" >&2
    exit 77
  fi
}

TMPDIR=${TMPDIR:-/tmp}
WORKDIR=$(mktemp -d "${TMPDIR%/}/fswatch-filter-root.XXXXXX")
PID=
CHECK_PID=

cleanup() {
  if [ -n "${CHECK_PID}" ]; then
    kill "${CHECK_PID}" 2>/dev/null || true
    wait "${CHECK_PID}" 2>/dev/null || true
  fi

  if [ -n "${PID}" ]; then
    kill "${PID}" 2>/dev/null || true
    wait "${PID}" 2>/dev/null || true
  fi

  rm -rf "${WORKDIR}"
}

trap cleanup EXIT INT TERM

if [ "${MONITOR}" = "fsevents_monitor" ]; then
  check_fsevents_available
fi

TESTDIR="${WORKDIR}/watched-root"
mkdir "${TESTDIR}"

"${FSWATCH}" -m "${MONITOR}" -r --format '%p %f' --event Created \
  -e '.*' -i '\.txt$' "${TESTDIR}" \
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

mkdir "${TESTDIR}/nested"
sleep 1

echo ignored > "${TESTDIR}/ignored.bin"
echo matching > "${TESTDIR}/matching.txt"
echo nested-ignored > "${TESTDIR}/nested/ignored.bin"
echo nested-matching > "${TESTDIR}/nested/matching.txt"

assert_event() {
  pattern=$1
  description=$2
  found=
  attempt=0

  while [ "${attempt}" -lt 10 ]; do
    if grep -Eq "${pattern}" "${WORKDIR}/out.log"; then
      found=1
      break
    fi

    attempt=$((attempt + 1))
    sleep 1
  done

  if [ -z "${found}" ]; then
    echo "missing ${description}" >&2
    echo "--- fswatch output ---" >&2
    sed -n '1,160p' "${WORKDIR}/out.log" >&2
    echo "--- fswatch stderr ---" >&2
    sed -n '1,80p' "${WORKDIR}/err.log" >&2
    exit 1
  fi
}

assert_event '/matching\.txt .*Created' 'matching child event through excluded root path'
assert_event '/nested/matching\.txt .*Created' 'matching nested child event through excluded directory path'

sleep 1

if grep -Eq 'ignored\.bin' "${WORKDIR}/out.log"; then
  echo "nonmatching child escaped path filters" >&2
  echo "--- fswatch output ---" >&2
  sed -n '1,160p' "${WORKDIR}/out.log" >&2
  echo "--- fswatch stderr ---" >&2
  sed -n '1,80p' "${WORKDIR}/err.log" >&2
  exit 1
fi
