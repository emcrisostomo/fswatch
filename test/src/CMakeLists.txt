#
# Copyright (c) 2014-2021 Enrico M. Crisostomo
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
set(EXEC_NAME "${PROJECT_NAME}_test")
include_directories(../.. .)

set(Header_Files "${PROJECT_BINARY_DIR}/libfswatch/libfswatch_config.h")
source_group("${EXEC_NAME} Header Files" FILES "${Header_Files}")

set(Source_Files "${EXEC_NAME}.c")
source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")
target_include_directories("${EXEC_NAME}" PRIVATE "${PROJECT_BINARY_DIR}")

target_link_libraries("${EXEC_NAME}" LINK_PUBLIC "libfswatch")
