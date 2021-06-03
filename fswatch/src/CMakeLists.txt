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
set(FSWATCH_SRC_FILES
        fswatch.cpp
        fswatch.hpp
        gettext.h
        ${PROJECT_BINARY_DIR}/cmake_config.h)

add_executable(fswatch ${FSWATCH_SRC_FILES})

# check for gettext and libintl
if (USE_NLS)
    if (Intl_LIBRARIES)
        target_link_libraries(fswatch PRIVATE ${Intl_LIBRARIES})
    endif ()
endif ()

target_include_directories(fswatch PUBLIC ../.. .)
target_include_directories(fswatch PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(fswatch PUBLIC libfswatch)

install(TARGETS fswatch DESTINATION bin)
