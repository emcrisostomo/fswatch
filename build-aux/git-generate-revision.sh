#!/bin/sh

REVISION_COUNT=$(git log --oneline | wc -l)

printf ${REVISION_COUNT}
