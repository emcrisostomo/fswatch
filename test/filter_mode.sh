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
WORKDIR=$(mktemp -d "${TMPDIR%/}/fswatch-filter-mode.XXXXXX")
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

assert_event() {
  pattern=$1
  description=$2
  output=$3
  error=$4
  found=
  attempt=0

  while [ "${attempt}" -lt 10 ]; do
    if grep -Eq "${pattern}" "${output}"; then
      found=1
      break
    fi

    attempt=$((attempt + 1))
    sleep 1
  done

  if [ -z "${found}" ]; then
    echo "missing ${description}" >&2
    echo "--- fswatch output ---" >&2
    sed -n '1,160p' "${output}" >&2
    echo "--- fswatch stderr ---" >&2
    sed -n '1,80p' "${error}" >&2
    exit 1
  fi
}

assert_no_event() {
  pattern=$1
  description=$2
  output=$3
  error=$4

  sleep 1

  if grep -Eq "${pattern}" "${output}"; then
    echo "unexpected ${description}" >&2
    echo "--- fswatch output ---" >&2
    sed -n '1,160p' "${output}" >&2
    echo "--- fswatch stderr ---" >&2
    sed -n '1,80p' "${error}" >&2
    exit 1
  fi
}

start_fswatch() {
  output=$1
  error=$2
  shift 2

  "${FSWATCH}" -m "${MONITOR}" -r --format '%p %f' --event Created \
    "$@" > "${output}" 2> "${error}" &
  PID=$!

  sleep 1

  if ! kill -0 "${PID}" 2>/dev/null; then
    if [ "${MONITOR}" = "fanotify_monitor" ] &&
       grep -Eqi 'fanotify|permission|operation not permitted|not supported|Cannot initialize' "${error}"; then
      echo "fanotify is unavailable in this environment" >&2
      sed -n '1,120p' "${error}" >&2
      exit 77
    fi

    echo "${MONITOR} exited unexpectedly" >&2
    sed -n '1,120p' "${error}" >&2
    exit 1
  fi
}

stop_fswatch() {
  if [ -n "${PID}" ]; then
    kill "${PID}" 2>/dev/null || true
    wait "${PID}" 2>/dev/null || true
    PID=
  fi
}

run_case() {
  name=$1
  shift
  TESTDIR="${WORKDIR}/${name}"
  mkdir "${TESTDIR}"
  OUT="${WORKDIR}/${name}.out"
  ERR="${WORKDIR}/${name}.err"

  start_fswatch "${OUT}" "${ERR}" "$@" "${TESTDIR}"

  echo keep > "${TESTDIR}/keep.c"
  echo skip > "${TESTDIR}/skip.c"
  echo other > "${TESTDIR}/other.txt"

  assert_event '/keep\.c .*Created' "${name}: included event" "${OUT}" "${ERR}"
}

if [ "${MONITOR}" = "fsevents_monitor" ]; then
  check_fsevents_available
fi

if "${FSWATCH}" --filter-mode=invalid "${WORKDIR}" > "${WORKDIR}/invalid.out" 2> "${WORKDIR}/invalid.err"; then
  echo "invalid filter mode was accepted" >&2
  exit 1
fi
if ! grep -Eq 'Invalid filter mode' "${WORKDIR}/invalid.err"; then
  echo "invalid filter mode did not explain the error" >&2
  sed -n '1,80p' "${WORKDIR}/invalid.err" >&2
  exit 1
fi

run_case legacy -e '.*' -i '\.c$' -e '/skip\.c$'
assert_event '/skip\.c .*Created' 'legacy include overrides exclude' "${OUT}" "${ERR}"
assert_no_event '/other\.txt .*Created' 'legacy excluded unmatched event' "${OUT}" "${ERR}"
stop_fswatch

run_case conjunctive --filter-mode=conjunctive -i '\.c$' -e '/skip\.c$'
assert_no_event '/skip\.c .*Created' 'conjunctive excluded included event' "${OUT}" "${ERR}"
assert_no_event '/other\.txt .*Created' 'conjunctive unmatched event' "${OUT}" "${ERR}"
stop_fswatch

run_case include-only --filter-mode=conjunctive -i '\.c$'
assert_event '/skip\.c .*Created' 'conjunctive included event' "${OUT}" "${ERR}"
assert_no_event '/other\.txt .*Created' 'conjunctive include-only unmatched event' "${OUT}" "${ERR}"
stop_fswatch

run_case exclude-only --filter-mode=conjunctive -e '/skip\.c$'
assert_no_event '/skip\.c .*Created' 'conjunctive exclude-only event' "${OUT}" "${ERR}"
assert_event '/other\.txt .*Created' 'conjunctive exclude-only unmatched event' "${OUT}" "${ERR}"
stop_fswatch
