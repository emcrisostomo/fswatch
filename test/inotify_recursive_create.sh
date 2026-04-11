#!/bin/sh

set -eu

if [ "$#" -ne 2 ]; then
  echo "usage: $0 FSWATCH GIT" >&2
  exit 2
fi

FSWATCH=$1
GIT=$2

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
