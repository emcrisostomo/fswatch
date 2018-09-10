#
# Copyright (c) 2014-2018 Enrico M. Crisostomo
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
include_directories(../.. .)
add_definitions(-DHAVE_CONFIG_H)

set(SOURCE_FILES
        fswatch_test.c
        ${PROJECT_BINARY_DIR}/cmake_config.h)

add_executable(fswatch_test ${SOURCE_FILES})
target_link_libraries(fswatch_test LINK_PUBLIC libfswatch)