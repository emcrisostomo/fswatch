#!/bin/sh
# -*- coding: utf-8; mode: sh; indent-tabs-mode: nil; tab-width: 2; sh-basic-offset: 2; sh-indentation: 2; fill-column: 75; -*-

REVISION_COUNT=$(git log --oneline | wc -l)

printf ${REVISION_COUNT}
