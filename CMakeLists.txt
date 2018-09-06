#
# Copyright (c) 2014-2016 Enrico M. Crisostomo
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
cmake_minimum_required(VERSION 3.8)
project(fswatch VERSION 1.13.0 LANGUAGES C CXX)

#@formatter:off
set(PACKAGE           "fswatch")
set(PACKAGE_NAME      "${PACKAGE}")
set(PACKAGE_VERSION   "${PROJECT_VERSION}-develop")
set(PACKAGE_STRING    "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_AUTHOR    "enrico.m.crisostomo@gmail.com")
set(PACKAGE_BUGREPORT "${PACKAGE_AUTHOR}")
set(PACKAGE_TARNAME   "${PACKAGE}")
set(PACKAGE_URL       "https://github.com/emcrisostomo/${PACKAGE}")
set(LOCALEDIR         "${CMAKE_INSTALL_PREFIX}/share/locale" CACHE FILEPATH "locale dir")
#@formatter:on

set(CMAKE_CXX_STANDARD 11)

# include modules
include(FindGettext)
include(FindIntl)
include(CheckIncludeFileCXX)
include(CheckStructHasMember)
include(CheckSymbolExists)

# checks
check_struct_has_member("struct stat" st_mtime sys/stat.h HAVE_STRUCT_STAT_ST_MTIME)
check_struct_has_member("struct stat" st_mtimespec sys/stat.h HAVE_STRUCT_STAT_ST_MTIMESPEC)

# Add a configuration file processed by cmake
configure_file(${PROJECT_SOURCE_DIR}/cmake_config.h.in ${PROJECT_BINARY_DIR}/cmake_config.h)
add_definitions(-DHAVE_CMAKE_CONFIG_H)

add_subdirectory(libfswatch)
add_subdirectory(fswatch/src)
add_subdirectory(test/src)
