#!/bin/sh
# -*- coding: utf-8; mode: sh; indent-tabs-mode: nil; tab-width: 2; sh-basic-offset: 2; sh-indentation: 2; fill-column: 75; -*-

LONG_VERSION=$(git describe --tags --long)
SHORT_VERSION=${LONG_VERSION%%-*}

printf ${SHORT_VERSION}
