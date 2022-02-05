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

set(LIBRARY_NAME "gettext")

# Define symbols to conditionally define FSEvents flags
if (APPLE)
    if ("${CMAKE_SYSTEM_VERSION}" VERSION_GREATER_EQUAL "9.0")
        set(HAVE_MACOS_GE_10_5 1)
    endif ()

    if ("${CMAKE_SYSTEM_VERSION}" VERSION_GREATER_EQUAL "11.0")
        set(HAVE_MACOS_GE_10_7 1)
    endif ()

    if ("${CMAKE_SYSTEM_VERSION}" VERSION_GREATER_EQUAL "13.0")
        set(HAVE_MACOS_GE_10_9 1)
    endif ()

    if ("${CMAKE_SYSTEM_VERSION}" VERSION_GREATER_EQUAL "14.0")
        set(HAVE_MACOS_GE_10_10 1)
    endif ()
endif (APPLE)

set(Header_Files "gettext.h" "gettext_defs.h")

check_include_file_cxx(mutex HAVE_CXX_MUTEX)

if (NOT HAVE_CXX_MUTEX)
    message(WARNING "${PROJECT_NAME} is not thread-safe because required C++11 library classes are not available.")
endif (NOT HAVE_CXX_MUTEX)

check_include_file_cxx(atomic HAVE_CXX_ATOMIC)

if (NOT HAVE_CXX_ATOMIC)
    message(WARNING "<atomic> unavailable: some functionalities will not be built.")
endif (NOT HAVE_CXX_ATOMIC)

check_cxx_symbol_exists(modf "math.h" HAVE_MODF)
check_cxx_symbol_exists(realpath cstdlib HAVE_REALPATH)
check_cxx_symbol_exists(select "sys/select.h" HAVE_SELECT)
check_struct_has_member("struct stat" st_mtime "sys/stat.h" HAVE_STRUCT_STAT_ST_MTIME)
check_struct_has_member("struct stat" st_mtimespec "sys/stat.h" HAVE_STRUCT_STAT_ST_MTIMESPEC)
check_include_file_cxx(unordered_map HAVE_UNORDERED_MAP)
check_include_file_cxx(unordered_set HAVE_UNORDERED_SET)

if (cxx_thread_local IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set(HAVE_CXX_THREAD_LOCAL ON CACHE BOOL "Enable C++ thread_local support")
endif (cxx_thread_local IN_LIST CMAKE_CXX_COMPILE_FEATURES)

source_group("${LIBRARY_NAME} Header Files" FILES "${Header_Files}")

add_library("${LIBRARY_NAME}" INTERFACE "${Header_Files}")
set_target_properties("${LIBRARY_NAME}" PROPERTIES PREFIX "")

# check for gettext and libintl
if (USE_NLS AND Intl_LIBRARIES)
    set(intl_nls_libs "${Intl_LIBRARIES}")
    if (VCPKG_TOOLCHAIN)
        find_package(Iconv REQUIRED)
        list(APPEND intl_nls_libs "Iconv::Charset" "Iconv::Iconv")
    endif (VCPKG_TOOLCHAIN)
    target_link_libraries("${LIBRARY_NAME}" INTERFACE "${intl_nls_libs}")
endif (USE_NLS AND Intl_LIBRARIES)
target_include_directories("${LIBRARY_NAME}" INTERFACE "${Intl_INCLUDE_DIRS}")

target_include_directories(
        "${LIBRARY_NAME}"
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:include>"
)
set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

set(installable_libs "${LIBRARY_NAME}")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif (TARGET "${DEPENDANT_LIBRARY}")
install(TARGETS ${installable_libs}
        DESTINATION "lib"
        EXPORT "${LIBRARY_NAME}Targets")
install(FILES ${Header_Files}
        DESTINATION "include")
install(EXPORT "${LIBRARY_NAME}Targets" DESTINATION "share/libfswatch")
