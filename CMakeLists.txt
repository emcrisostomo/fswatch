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
cmake_minimum_required(VERSION 3.8)
project(fs_watch VERSION 1.17.0 LANGUAGES C CXX)

#@formatter:off
set(PACKAGE           "fswatch")
set(PACKAGE_NAME      "${PACKAGE}")
set(PACKAGE_VERSION   "${PROJECT_VERSION}-develop")
set(PACKAGE_STRING    "${PACKAGE} ${PACKAGE_VERSION}")
set(PACKAGE_AUTHOR    "enrico.m.crisostomo@gmail.com")
set(PACKAGE_BUGREPORT "${PACKAGE_AUTHOR}")
set(PACKAGE_TARNAME   "${PACKAGE}")
set(PACKAGE_URL       "https://github.com/emcrisostomo/${PACKAGE}")
set(LOCALEDIR         "${CMAKE_INSTALL_PREFIX}/share/locale" CACHE FILEPATH "locale dir")
#@formatter:on

set(CMAKE_CXX_STANDARD 11)
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif (MSVC)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

# Add option to choose between shared and static libraries
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# include modules
include(FindGettext)
include(FindIntl)
include(CheckIncludeFileCXX)
include(CheckStructHasMember)
include(CheckCXXSymbolExists)

# check for gettext and libintl
check_include_file_cxx(getopt.h HAVE_GETOPT_H)

if (HAVE_GETOPT_H)
    check_cxx_symbol_exists(getopt_long "getopt.h" HAVE_GETOPT_LONG)
else ()
    set(HAVE_GETOPT_H 1)
    set(HAVE_GETOPT_LONG 1)
endif (HAVE_GETOPT_H)

# If both gettext and libintl are found, define the USE_NLS variable to
# optionally disable them
if (Intl_FOUND AND GETTEXT_FOUND)
    option(USE_NLS "Use NLS" ON)
endif ()

if (USE_NLS)
    set(ENABLE_NLS 1)

    # gettext configuration
    set(LINGUAS en en@quot en@boldquot it es)

    foreach (language ${LINGUAS})
        configure_file("${PROJECT_SOURCE_DIR}/po/${language}.po" "${PROJECT_BINARY_DIR}/${language}.po" COPYONLY)
    endforeach ()

    gettext_process_pot_file("po/${PACKAGE}.pot" ALL INSTALL_DESTINATION "${LOCALEDIR}" LANGUAGES ${LINGUAS})
endif ()

# checks
check_cxx_symbol_exists(atexit cstdlib HAVE_ATEXIT)
check_cxx_symbol_exists(setlocale clocale HAVE_SETLOCALE)
check_cxx_symbol_exists(strtod cstdlib HAVE_STRTOD)

option(BUILD_FSWATCH "Build ${PROJECT_NAME}" ON)
option(BUILD_TESTS "Build ${PROJECT_NAME} tests" ON)

add_subdirectory("libfswatch")
if (BUILD_FSWATCH)
    add_subdirectory("fswatch/src")
endif (BUILD_FSWATCH)
if (BUILD_TESTS)
    add_subdirectory("test/src")
endif (BUILD_TESTS)
