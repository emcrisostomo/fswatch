#!/bin/zsh
# -*- vim:fenc=utf-8:et:sw=2:ts=2:sts=2
#
# Copyright (c) 2025 Enrico M. Crisostomo
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
typeset -i ARGS_PROCESSED=0
typeset -a develop_flag
typeset -a help_flag
typeset -a version_string
typeset -a version_flag
DEVELOP=0
REQUIRED_PROGS=( git semver)

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
  print -- "Copyright (C) 2025 Enrico M. Crisostomo"
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
  print -- "${PROGNAME} [-s|--set version]* [-d|--develop]*"
  print -- "${PROGNAME} [-h|--help]"
  print -- "${PROGNAME} [--version]"
  print
  print -- "Bumps the project version in all the relevant files:"
  print -- " - m4/fswatch_version.m4"
  print -- " - m4/libfswatch_version.m4"
  print -- " - CMakeLists.txt"
  print
  print -- "The current maximum version is found by inspecting git tags."
  print -- "The maximum version is then bumped by one minor version and the"
  print -- "-develop suffix is appended if the program is invoked with"
  print -- "-d/--develop.  The new version is then written to the files listed"
  print -- "above."
  print
  print -- "Options:"
  print -- " -d, --develop  The -develop suffix is appended."
  print -- " -h, --help     Print this message."
  print -- " -s, --set      Set the version to the specified value."
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
           d=develop_flag -develop=develop_flag \
           h=help_flag -help=help_flag \
           s+:=version_string --set=version_string \
           -version=version_flag

if (( ${+help_flag[1]} > 0 ))
then
  print_usage
  exit 0
fi

if (( ${+develop_flag[1]} > 0 ))
then
  DEVELOP=1
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

if (( ${+version_string[1]} > 0 ))
then
  (( ${#version_string} == 2)) ||
    {
      >&2 print -- "Invalid number of version strings."
      exit 1
    }
  NEW_VERSION=${version_string[2]}
else
  NEW_VERSION=$(git tag | semver -sr 2> /dev/null | semver --max | semver -b minor)
fi 

if (( ${DEVELOP} > 0 ))
then
  VERSION_MODIFIER=-develop
else
  VERSION_MODIFIER=
fi

echo "Bumping version to ${NEW_VERSION}"

# Updating m4/libfswatch_version.m4
FILE_PATH=${PROGDIR}/m4/libfswatch_version.m4
sed -i.bak "s/\(m4_define(\[LIBFSWATCH_VERSION\], \[\)[^]]*\(\]\)/\1${NEW_VERSION}${VERSION_MODIFIER}\2/" ${FILE_PATH}
rm ${FILE_PATH}.bak

# Updating m4/fswatch_version.m4
FILE_PATH=${PROGDIR}/m4/fswatch_version.m4
sed -i.bak "s/\(m4_define(\[FSWATCH_VERSION\], \[\)[^]]*\(\]\)/\1${NEW_VERSION}${VERSION_MODIFIER}\2/" ${FILE_PATH}
rm ${FILE_PATH}.bak

# Updating CMakeLists.txt
FILE_PATH=${PROGDIR}/CMakeLists.txt
sed -i.bak "s/\(project(fswatch VERSION *\)[^ ]*/\1${NEW_VERSION}/" ${FILE_PATH}
sed -i.bak "s/\(set(VERSION_MODIFIER *\"\)[^\"]*\(\")\)/\1${VERSION_MODIFIER}\2/" ${FILE_PATH}
rm ${FILE_PATH}.bak

# Local variables:
# coding: utf-8
# mode: sh
# eval: (sh-set-shell "zsh")
# tab-width: 2
# indent-tabs-mode: nil
# sh-basic-offset: 2
# sh-indentation: 2
# End:
