#
# Copyright (c) 2014-2025 Enrico M. Crisostomo
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

# Define symbols to conditionally define FSEvents flags
if(APPLE)
    if (${CMAKE_SYSTEM_VERSION} VERSION_GREATER_EQUAL "9.0")
        set(HAVE_MACOS_GE_10_5 1)
    endif()

    if (${CMAKE_SYSTEM_VERSION} VERSION_GREATER_EQUAL "10.0")
        set(HAVE_MACOS_GE_10_6 1)
    endif()

    if (${CMAKE_SYSTEM_VERSION} VERSION_GREATER_EQUAL "11.0")
        set(HAVE_MACOS_GE_10_7 1)
    endif()

    if (${CMAKE_SYSTEM_VERSION} VERSION_GREATER_EQUAL "13.0")
        set(HAVE_MACOS_GE_10_9 1)
    endif()

    if (${CMAKE_SYSTEM_VERSION} VERSION_GREATER_EQUAL "14.0")
        set(HAVE_MACOS_GE_10_10 1)
    endif()

    if (${CMAKE_SYSTEM_VERSION} VERSION_GREATER_EQUAL "17.0")
        set(HAVE_MACOS_GE_10_13 1)
    endif()
endif()

set(LIBFSWATCH_HEADER_FILES
        src/libfswatch/c/cevent.h
        src/libfswatch/c/cfilter.h
        src/libfswatch/c/cmonitor.h
        src/libfswatch/c/error.h
        src/libfswatch/c/libfswatch.h
        src/libfswatch/c/libfswatch_log.h
        src/libfswatch/c/libfswatch_types.h
        src/libfswatch/c++/event.hpp
        src/libfswatch/c++/filter.hpp
        src/libfswatch/c++/libfswatch_exception.hpp
        src/libfswatch/c++/monitor.hpp
        src/libfswatch/c++/monitor_factory.hpp
        src/libfswatch/c++/path_utils.hpp
        src/libfswatch/c++/poll_monitor.hpp
        src/libfswatch/c++/string/string_utils.hpp
        src/libfswatch/gettext.h
        src/libfswatch/gettext_defs.h
        ${CMAKE_CURRENT_BINARY_DIR}/libfswatch_config.h)

set(LIB_SOURCE_FILES
        src/libfswatch/c/cevent.cpp
        src/libfswatch/c/libfswatch.cpp
        src/libfswatch/c/libfswatch_log.cpp
        src/libfswatch/c++/event.cpp
        src/libfswatch/c++/filter.cpp
        src/libfswatch/c++/libfswatch_exception.cpp
        src/libfswatch/c++/monitor.cpp
        src/libfswatch/c++/monitor_factory.cpp
        src/libfswatch/c++/path_utils.cpp
        src/libfswatch/c++/poll_monitor.cpp
        src/libfswatch/c++/string/string_utils.cpp)

check_struct_has_member("struct stat" st_mtime sys/stat.h HAVE_STRUCT_STAT_ST_MTIME)
check_struct_has_member("struct stat" st_mtimespec sys/stat.h HAVE_STRUCT_STAT_ST_MTIMESPEC)

check_include_file_cxx(sys/inotify.h HAVE_SYS_INOTIFY_H)

if (HAVE_SYS_INOTIFY_H)
    set(LIBFSWATCH_HEADER_FILES
            ${LIBFSWATCH_HEADER_FILES}
            src/libfswatch/c++/inotify_monitor.hpp)
    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            src/libfswatch/c++/inotify_monitor.cpp)
endif (HAVE_SYS_INOTIFY_H)

check_include_file_cxx(sys/event.h HAVE_SYS_EVENT_H)

if (HAVE_SYS_EVENT_H)
    set(LIBFSWATCH_HEADER_FILES
            ${LIBFSWATCH_HEADER_FILES}
            src/libfswatch/c++/kqueue_monitor.hpp)
    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            src/libfswatch/c++/kqueue_monitor.cpp)
endif (HAVE_SYS_EVENT_H)

check_include_file_cxx(port.h HAVE_PORT_H)

if (HAVE_PORT_H)
    set(LIBFSWATCH_HEADER_FILES
            ${LIBFSWATCH_HEADER_FILES}
            src/libfswatch/c++/fen_monitor.hpp)
    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            src/libfswatch/c++/fen_monitor.cpp)
endif (HAVE_PORT_H)

check_cxx_symbol_exists(FindFirstChangeNotification windows.h HAVE_WINDOWS_HEADER)

if (HAVE_WINDOWS_HEADER)

    set(LIBFSWATCH_HEADER_FILES
            ${LIBFSWATCH_HEADER_FILES}
            src/libfswatch/c++/windows/win_directory_change_event.hpp
            src/libfswatch/c++/windows/win_error_message.hpp
            src/libfswatch/c++/windows/win_handle.hpp
            src/libfswatch/c++/windows/win_paths.hpp
            src/libfswatch/c++/windows/win_strings.hpp
            src/libfswatch/c++/windows_monitor.hpp)

    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            src/libfswatch/c++/windows/win_directory_change_event.cpp
            src/libfswatch/c++/windows/win_error_message.cpp
            src/libfswatch/c++/windows/win_handle.cpp
            src/libfswatch/c++/windows/win_paths.cpp
            src/libfswatch/c++/windows/win_strings.cpp
            src/libfswatch/c++/windows_monitor.cpp)
    set(HAVE_WINDOWS ON CACHE BOOL "Enable Windows support")

endif (HAVE_WINDOWS_HEADER)

if (APPLE)
    check_include_file_cxx(CoreServices/CoreServices.h HAVE_FSEVENTS_FILE_EVENTS)

    if (HAVE_FSEVENTS_FILE_EVENTS)
        find_library(CORESERVICES_LIBRARY CoreServices)

        if (CORESERVICES_LIBRARY)
            set(CMAKE_REQUIRED_LIBRARIES ${CORESERVICES_LIBRARY})
            check_symbol_exists(FSEventStreamSetDispatchQueue "CoreServices/CoreServices.h" HAVE_FSEVENTS_FSEVENTSTREAMSETDISPATCHQUEUE)

            if (HAVE_FSEVENTS_FSEVENTSTREAMSETDISPATCHQUEUE)
                set(EXTRA_LIBS ${EXTRA_LIBS} ${CORESERVICES_LIBRARY})
                set(LIBFSWATCH_HEADER_FILES
                    ${LIBFSWATCH_HEADER_FILES}
                    src/libfswatch/c++/fsevents_monitor.hpp)
                set(LIB_SOURCE_FILES
                    ${LIB_SOURCE_FILES}
                    src/libfswatch/c++/fsevents_monitor.cpp)
            endif (HAVE_FSEVENTS_FSEVENTSTREAMSETDISPATCHQUEUE)
        endif ()
    endif (HAVE_FSEVENTS_FILE_EVENTS)
endif (APPLE)

# Add a configuration file processed by cmake
configure_file(libfswatch_config.in libfswatch_config.h)
# TODO: consider removing it
add_definitions(-DHAVE_LIBFSWATCH_CONFIG_H)

add_library(libfswatch ${LIB_SOURCE_FILES} ${LIBFSWATCH_HEADER_FILES})
set_target_properties(libfswatch PROPERTIES PREFIX "")

# check for gettext and libintl
if (USE_NLS)
    if (Intl_LIBRARIES)
        target_link_libraries(libfswatch PRIVATE ${Intl_LIBRARIES})
    endif ()
endif ()

target_include_directories(libfswatch PUBLIC src/libfswatch)
target_include_directories(libfswatch PUBLIC src)
target_include_directories(libfswatch PUBLIC ${Intl_INCLUDE_DIRS})
target_include_directories(libfswatch PRIVATE ${PROJECT_BINARY_DIR})
target_include_directories(libfswatch PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(libfswatch PRIVATE ${EXTRA_LIBS})

install(TARGETS libfswatch LIBRARY DESTINATION lib)
# TODO: should migrate to target_source(file_set) to install headers
install(DIRECTORY src/libfswatch
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.hpp"
    PATTERN "*.h")
