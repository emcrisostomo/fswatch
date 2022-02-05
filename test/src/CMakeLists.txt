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
set(EXEC_NAME "${PACKAGE}_test")

set(Header_Files "${PROJECT_BINARY_DIR}/libfswatch/libfswatch_config.h")
source_group("${EXEC_NAME} Header Files" FILES "${Header_Files}")

set(Source_Files "${EXEC_NAME}.c")
source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")

if (MSVC)
    find_package(pthreads QUIET)
    if (PTHREADS4W_FOUND)
        set(PTHREAD_LIBRARY "PThreads4W::PThreads4W")
    else ()
        include("${CMAKE_SOURCE_DIR}/CMake/FindPthreads.cmake")
        set(PTHREAD_LIBRARY ${PTHREADS_LIBRARIES})
    endif (PTHREADS4W_FOUND)

    # set(EXTRA_LIBS ${EXTRA_LIBS} "advapi32")
else ()
    find_library(PTHREAD_LIBRARY pthread)
endif (MSVC)
target_link_libraries("${EXEC_NAME}" PRIVATE "${PTHREAD_LIBRARY}")

set_target_properties(
        "${EXEC_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

target_link_libraries("${EXEC_NAME}" LINK_PUBLIC "libfswatch")
