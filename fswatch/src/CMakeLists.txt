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
set(EXEC_NAME "fswatch_bin")
set(Header_Files "${PACKAGE}.hpp" "gettext.h")
source_group("${PACKAGE} Header Files" FILES "${Header_Files}")

set(Source_Files "${PACKAGE}.cpp")
source_group("${PACKAGE} Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")

set_target_properties(
        "${EXEC_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        CXX
)

if (MSVC)
    add_subdirectory("./windows")
    target_link_libraries("${EXEC_NAME}" PRIVATE "getopt")
endif (MSVC)

target_link_libraries("${EXEC_NAME}" PUBLIC "libfswatch")

if (DEFINED VCPKG_LIBRARY_LINKAGE)
    if (VCPKG_LIBRARY_LINKAGE STREQUAL "static")
        set(BUILD_IS_STATIC "ON")
    endif (VCPKG_LIBRARY_LINKAGE STREQUAL "static")
elseif (BUILD_SHARED_LIBS)
    set(BUILD_IS_STATIC "OFF")
endif ()

if (BUILD_IS_STATIC STREQUAL "ON")
    set(bin_install_dir "${CURRENT_PACKAGES_DIR}")
else ()
    install(FILES ${Header_Files} DESTINATION "include")
    set(bin_install_dir "bin")
endif ()
install(TARGETS "${EXEC_NAME}" DESTINATION "${bin_install_dir}")
