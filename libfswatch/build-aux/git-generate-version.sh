#!/bin/sh

LONG_VERSION=$(git describe --tags --long)
SHORT_VERSION=${LONG_VERSION%%-*}

printf ${SHORT_VERSION}
