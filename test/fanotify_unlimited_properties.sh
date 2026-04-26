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
WORKDIR=$(mktemp -d "${TMPDIR%/}/fswatch-fanotify-unlimited.XXXXXX")
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

start_and_expect_create() {
  : > "${WORKDIR}/out.log"
  : > "${WORKDIR}/err.log"

  "${FSWATCH}" -m fanotify_monitor -r -l 1 \
    --monitor-property fanotify.unlimited-queue=false \
    --monitor-property fanotify.unlimited-marks=false \
    --format '%p %f' "${TESTDIR}" \
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

  echo content > "${TESTDIR}/false-properties.txt"
  sleep 2

  kill "${PID}" 2>/dev/null || true
  wait "${PID}" 2>/dev/null || true
  PID=

  if ! grep -q 'false-properties\.txt.*Created' "${WORKDIR}/out.log"; then
    echo "missing create event with explicit false unlimited properties" >&2
    sed -n '1,160p' "${WORKDIR}/out.log" >&2
    sed -n '1,160p' "${WORKDIR}/err.log" >&2
    exit 1
  fi
}

start_unlimited_or_expect_clear_error() {
  : > "${WORKDIR}/out.log"
  : > "${WORKDIR}/err.log"

  "${FSWATCH}" -m fanotify_monitor -r -l 1 \
    --monitor-property fanotify.unlimited-queue=true \
    --monitor-property fanotify.unlimited-marks=true \
    --format '%p %f' "${TESTDIR}" \
    > "${WORKDIR}/out.log" 2> "${WORKDIR}/err.log" &
  PID=$!

  sleep 1

  if ! kill -0 "${PID}" 2>/dev/null; then
    wait "${PID}" 2>/dev/null || true
    PID=

    if grep -Eq 'requires (CAP_SYS_ADMIN|FAN_UNLIMITED_(QUEUE|MARKS))' "${WORKDIR}/err.log"; then
      return
    fi

    if grep -Eqi 'fanotify|permission|operation not permitted|not supported|Cannot initialize' "${WORKDIR}/err.log"; then
      echo "fanotify is unavailable in this environment" >&2
      sed -n '1,120p' "${WORKDIR}/err.log" >&2
      exit 77
    fi

    echo "fanotify monitor exited unexpectedly" >&2
    sed -n '1,120p' "${WORKDIR}/err.log" >&2
    exit 1
  fi

  echo content > "${TESTDIR}/true-properties.txt"
  sleep 2

  kill "${PID}" 2>/dev/null || true
  wait "${PID}" 2>/dev/null || true
  PID=

  if ! grep -q 'true-properties\.txt.*Created' "${WORKDIR}/out.log"; then
    echo "missing create event with true unlimited properties" >&2
    sed -n '1,160p' "${WORKDIR}/out.log" >&2
    sed -n '1,160p' "${WORKDIR}/err.log" >&2
    exit 1
  fi
}

start_and_expect_create
start_unlimited_or_expect_clear_error
