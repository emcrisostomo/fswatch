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
mkdir "${TESTDIR}"

"${FSWATCH}" -m fanotify_monitor -r --format '%p %f' "${TESTDIR}" \
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

CREATE_FILE="${TESTDIR}/fanotify-create-file.txt"
MODIFY_FILE="${TESTDIR}/fanotify-modify-file.txt"
RENAME_SOURCE="${TESTDIR}/fanotify-rename-source.txt"
RENAME_TARGET="${TESTDIR}/fanotify-rename-target.txt"
DELETE_FILE="${TESTDIR}/fanotify-delete-file.txt"
CREATED_DIR="${TESTDIR}/fanotify-created-dir"
RENAME_DIR_SOURCE="${TESTDIR}/fanotify-rename-dir-source"
RENAME_DIR_TARGET="${TESTDIR}/fanotify-rename-dir-target"
DELETE_DIR="${TESTDIR}/fanotify-delete-dir"
FAST_DIR="${TESTDIR}/fanotify-fast-dir"
FAST_CHILD="${FAST_DIR}/fanotify-recursive-child.txt"

echo created > "${CREATE_FILE}"

echo original > "${MODIFY_FILE}"
sleep 1
echo modified >> "${MODIFY_FILE}"

echo renamed > "${RENAME_SOURCE}"
sleep 1
mv "${RENAME_SOURCE}" "${RENAME_TARGET}"

echo deleted > "${DELETE_FILE}"
sleep 1
rm "${DELETE_FILE}"

mkdir "${CREATED_DIR}"

mkdir "${RENAME_DIR_SOURCE}"
sleep 1
mv "${RENAME_DIR_SOURCE}" "${RENAME_DIR_TARGET}"

mkdir "${DELETE_DIR}"
sleep 1
rmdir "${DELETE_DIR}"

mkdir "${FAST_DIR}"
echo child > "${FAST_CHILD}"

event_seen() {
  basename=$1
  flag=$2

  grep -E "${basename} .*${flag}" "${WORKDIR}/out.log" >/dev/null
}

wait_for_event() {
  basename=$1
  flag=$2
  attempt=0

  while [ "${attempt}" -lt 10 ]; do
    if event_seen "${basename}" "${flag}"; then
      return 0
    fi

    attempt=$((attempt + 1))
    sleep 1
  done

  return 1
}

assert_event() {
  basename=$1
  flag=$2
  description=$3

  if wait_for_event "${basename}" "${flag}"; then
    return 0
  fi

  echo "missing fanotify event: ${description}" >&2
  echo "--- fswatch output ---" >&2
  sed -n '1,240p' "${WORKDIR}/out.log" >&2
  echo "--- fswatch stderr ---" >&2
  sed -n '1,160p' "${WORKDIR}/err.log" >&2
  exit 1
}

assert_event 'fanotify-create-file\.txt' 'Created' 'file creation'
assert_event 'fanotify-modify-file\.txt' '(Updated|CloseWrite)' 'file modification'
assert_event 'fanotify-rename-source\.txt' 'MovedFrom' 'file rename source'
assert_event 'fanotify-rename-target\.txt' 'MovedTo' 'file rename target'
assert_event 'fanotify-delete-file\.txt' 'Removed' 'file deletion'
assert_event 'fanotify-created-dir' 'Created' 'subdirectory creation'
assert_event 'fanotify-created-dir' 'IsDir' 'subdirectory creation directory flag'
assert_event 'fanotify-rename-dir-source' 'MovedFrom' 'subdirectory rename source'
assert_event 'fanotify-rename-dir-target' 'MovedTo' 'subdirectory rename target'
assert_event 'fanotify-delete-dir' 'Removed' 'subdirectory deletion'
assert_event 'fanotify-recursive-child\.txt' 'Created' 'recursive child creation in a new subdirectory'
