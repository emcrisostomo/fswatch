#
# Copyright (c) 2014-2022 Enrico M. Crisostomo
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
set(SOURCE_FILES
        fswatch_test.cpp
        ${PROJECT_BINARY_DIR}/libfswatch/libfswatch_config.h)

add_executable(fswatch_test ${SOURCE_FILES})
target_include_directories(fswatch_test PRIVATE ../.. .)
target_include_directories(fswatch_test PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(fswatch_test PUBLIC libfswatch)