#!/bin/zsh
# -*- vim:fenc=utf-8:et:sw=2:ts=2:sts=2
#
# Copyright (c) 2014-2015 Enrico M. Crisostomo
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
#
setopt local_options
setopt local_traps
unsetopt glob_subst

set -o errexit
set -o nounset

PROGNAME=${0:t}
PROGDIR=${0:h}
BUG_REPORT=enrico.m.crisostomo@gmail.com
PACKAGE_VERSION=1.0.0
typeset -r LIBFSWATCH_DOC_DIR=libfswatch/doc/doxygen
typeset -r FSWATCH_DOC_DIR=fswatch/doc
typeset -i ARGS_PROCESSED=0
typeset -a help_flag
typeset -a verbose_flag
typeset -a version_flag
VERBOSE=0
typeset -a configure_opts
REQUIRED_PROGS=( doxygen git )

for p in ${REQUIRED_PROGS}
do
  command -v ${p} > /dev/null 2>&1 ||
    {
      >&2 print -- Cannot find required program: ${p}
      exit 1
    }
done

print_version()
{
  print -- "${PROGNAME} ${PACKAGE_VERSION}"
  print -- "Copyright (C) 2017 Enrico M. Crisostomo"
  print -- "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>."
  print -- "This is free software: you are free to change and redistribute it."
  print -- "There is NO WARRANTY, to the extent permitted by law."
  print
  print -- "Written by Enrico M. Crisostomo"
}

print_usage()
{
  print -- "${PROGNAME}"
  print
  print -- "Usage:"
  print -- "${PROGNAME}"
  print
  print -- "Build the package from scratch by performing the following steps:"
  print -- " - Run the maintainer-clean target."
  print -- " - Bootstrap the autotools."
  print -- " - Configure the package."
  print -- " - Run the distcheck target."
  print -- " - Run the maintainer-clean target."
  print
  print -- "Options:"
  print -- " -h, --help     Print this message."
  print -- " -v, --verbose  Print verbose output."
  print -- "     --version  Print the program version."
}

parse_opts()
{
  while getopts ":" opt
  do
    case $opt in
      \?)
        >&2 print -- Invalid option -${OPTARG}.
        exit 1
        ;;
    esac
  done

  ARGS_PROCESSED=$((OPTIND - 1))
}

# main
zparseopts -D \
           h=help_flag -help=help_flag \
           v=verbose_flag -verbose=verbose_flag \
           -version=version_flag

if (( ${+help_flag[1]} > 0 ))
then
  print_usage
  exit 0
fi

if (( ${+verbose_flag[1]} > 0 ))
then
  VERBOSE=1
fi

if (( ${+version_flag[1]} > 0 ))
then
  print_version
  exit 0
fi

parse_opts $* && shift ${ARGS_PROCESSED}

(( $# == 0 )) ||
  {
    >&2 print -- "Invalid number of arguments."
    exit 1
  }

if (( ${VERBOSE} > 0 ))
then
  configure_opts=( --enable-silent-rules )
else
  configure_opts=( --disable-silent-rules )
fi

cd ${PROGDIR}
make maintainer-clean || true
rm -rf dist
./autogen.sh
./configure ${configure_opts}
make -j distcheck
make -C fswatch html pdf
make -C ${LIBFSWATCH_DOC_DIR} doxygen
mkdir -p dist/fswatch/pdf
mkdir -p dist/libfswatch/pdf
cp -r ${LIBFSWATCH_DOC_DIR}/html             dist/libfswatch
cp    ${LIBFSWATCH_DOC_DIR}/latex/refman.pdf dist/libfswatch/pdf
cp -r ${FSWATCH_DOC_DIR}/fswatch.html        dist/fswatch
cp    ${FSWATCH_DOC_DIR}/fswatch.pdf         dist/fswatch/pdf
make maintainer-clean

# Local variables:
# coding: utf-8
# mode: sh
# eval: (sh-set-shell "zsh")
# tab-width: 2
# indent-tabs-mode: nil
# sh-basic-offset: 2
# sh-indentation: 2
# End:
