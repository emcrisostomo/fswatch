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
set(Header_Files "${PROJECT_NAME}.hpp" "gettext.h")
source_group("${PROJECT_NAME} Header Files" FILES "${Header_Files}")

set(Source_Files "${PROJECT_NAME}.cpp")
source_group("${PROJECT_NAME} Source Files" FILES "${Source_Files}")

add_executable("${PROJECT_NAME}" "${Header_Files}" "${Source_Files}")

# check for gettext and libintl
if (USE_NLS AND Intl_LIBRARIES)
    set(intl_nls_libs "${Intl_LIBRARIES}")
    if (VCPKG_TOOLCHAIN)
        include(FindIconv)
        find_package(Iconv REQUIRED)
        list(APPEND intl_nls_libs "Iconv::Iconv")
    endif (VCPKG_TOOLCHAIN)
    target_link_libraries("${PROJECT_NAME}" PRIVATE "${intl_nls_libs}")
endif (USE_NLS AND Intl_LIBRARIES)

if (MSVC)
    add_subdirectory("./windows")
    target_link_libraries("${PROJECT_NAME}" PUBLIC "getopt")
endif (MSVC)

target_include_directories("${PROJECT_NAME}" PUBLIC ../.. .)
target_include_directories("${PROJECT_NAME}" PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries("${PROJECT_NAME}" PUBLIC "libfswatch")

if (BUILD_SHARED_LIBS)
    install(FILES ${Header_Files} DESTINATION "include")
    set(bin_install_dir "bin")
else ()
    set(bin_install_dir "${CURRENT_PACKAGES_DIR}")
endif ()
install(TARGETS "${PROJECT_NAME}" DESTINATION "${bin_install_dir}")
