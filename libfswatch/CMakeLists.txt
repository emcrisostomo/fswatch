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

set(LIBRARY_NAME "libfswatch")

add_library("${LIBRARY_NAME}" INTERFACE)

add_subdirectory("./src/libfswatch/gettext")
target_link_libraries("${LIBRARY_NAME}" INTERFACE "gettext")

add_subdirectory("./src/libfswatch/c")
target_link_libraries("${LIBRARY_NAME}" INTERFACE "c${PACKAGE}")

add_subdirectory("./src/libfswatch/c++")
target_link_libraries("${LIBRARY_NAME}" INTERFACE "cxx${PACKAGE}")

if (NOT DEFINED TARGET_ARCH)
    # See https://gitlab.kitware.com/cmake/cmake/-/issues/15170 for why `CMAKE_SYSTEM_PROCESSOR` isn't used
    if (DEFINED CMAKE_C_COMPILER_ARCHITECTURE_ID)
        set(TARGET_ARCH "${CMAKE_C_COMPILER_ARCHITECTURE_ID}")
    elseif (DEFINED CMAKE_CXX_COMPILER_ARCHITECTURE_ID)
        set(TARGET_ARCH "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}")
    else ()
        set(TARGET_ARG "${CMAKE_SYSTEM_PROCESSOR}")
    endif ()
endif (NOT DEFINED TARGET_ARCH)

# Add a configuration file processed by cmake
configure_file("libfswatch_config.in" "${CMAKE_CURRENT_BINARY_DIR}/libfswatch_config.h")

set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        CXX
)

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "1.0.0")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "1")

install(TARGETS "${LIBRARY_NAME}" LIBRARY DESTINATION "lib")
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libfswatch_config.h"
        DESTINATION "include")
