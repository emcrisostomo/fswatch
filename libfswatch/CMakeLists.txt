#
# Copyright (c) 2014-2026 Enrico M. Crisostomo
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

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

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
check_include_file_cxx(sys/epoll.h HAVE_SYS_EPOLL_H)
check_include_file_cxx(sys/eventfd.h HAVE_SYS_EVENTFD_H)

if (HAVE_SYS_INOTIFY_H AND HAVE_SYS_EPOLL_H AND HAVE_SYS_EVENTFD_H)
    set(CMAKE_REQUIRED_DEFINITIONS_SAVED ${CMAKE_REQUIRED_DEFINITIONS})
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
    check_cxx_symbol_exists(inotify_init1 sys/inotify.h HAVE_INOTIFY_INIT1)
    check_cxx_symbol_exists(epoll_create1 sys/epoll.h HAVE_EPOLL_CREATE1)
    check_cxx_symbol_exists(eventfd sys/eventfd.h HAVE_EVENTFD)
    set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS_SAVED})
endif ()

if (HAVE_SYS_INOTIFY_H AND HAVE_SYS_EPOLL_H AND HAVE_SYS_EVENTFD_H AND
        HAVE_INOTIFY_INIT1 AND HAVE_EPOLL_CREATE1 AND HAVE_EVENTFD)
    set(HAVE_INOTIFY_MONITOR 1 CACHE INTERNAL "Linux inotify monitor is available")
    set(LIBFSWATCH_HEADER_FILES
            ${LIBFSWATCH_HEADER_FILES}
            src/libfswatch/c++/inotify_monitor.hpp)
    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            src/libfswatch/c++/inotify_monitor.cpp)
endif ()

check_include_file_cxx(sys/fanotify.h HAVE_SYS_FANOTIFY_H)
check_include_file_cxx(sys/epoll.h HAVE_SYS_EPOLL_H)
check_include_file_cxx(sys/eventfd.h HAVE_SYS_EVENTFD_H)

if (HAVE_SYS_FANOTIFY_H AND HAVE_SYS_EPOLL_H AND HAVE_SYS_EVENTFD_H)
    check_cxx_symbol_exists(fanotify_init sys/fanotify.h HAVE_FANOTIFY_INIT)
    check_cxx_symbol_exists(fanotify_mark sys/fanotify.h HAVE_FANOTIFY_MARK)
    check_cxx_symbol_exists(epoll_create1 sys/epoll.h HAVE_EPOLL_CREATE1)
    check_cxx_symbol_exists(epoll_ctl sys/epoll.h HAVE_EPOLL_CTL)
    check_cxx_symbol_exists(epoll_wait sys/epoll.h HAVE_EPOLL_WAIT)
    check_cxx_symbol_exists(eventfd sys/eventfd.h HAVE_EVENTFD)
    check_cxx_symbol_exists(FAN_REPORT_DFID_NAME sys/fanotify.h HAVE_FAN_REPORT_DFID_NAME)
    check_cxx_symbol_exists(FAN_CREATE sys/fanotify.h HAVE_FAN_CREATE)
    check_cxx_symbol_exists(FAN_DELETE sys/fanotify.h HAVE_FAN_DELETE)
    check_cxx_symbol_exists(FAN_MOVED_FROM sys/fanotify.h HAVE_FAN_MOVED_FROM)
    check_cxx_symbol_exists(FAN_MOVED_TO sys/fanotify.h HAVE_FAN_MOVED_TO)
    check_cxx_symbol_exists(FAN_EVENT_ON_CHILD sys/fanotify.h HAVE_FAN_EVENT_ON_CHILD)
    check_cxx_symbol_exists(FAN_Q_OVERFLOW sys/fanotify.h HAVE_FAN_Q_OVERFLOW)

    if (HAVE_FANOTIFY_INIT AND HAVE_FANOTIFY_MARK AND
        HAVE_EPOLL_CREATE1 AND HAVE_EPOLL_CTL AND HAVE_EPOLL_WAIT AND HAVE_EVENTFD AND
        HAVE_FAN_REPORT_DFID_NAME AND HAVE_FAN_CREATE AND HAVE_FAN_DELETE AND
        HAVE_FAN_MOVED_FROM AND HAVE_FAN_MOVED_TO AND
        HAVE_FAN_EVENT_ON_CHILD AND HAVE_FAN_Q_OVERFLOW)
        set(HAVE_FANOTIFY ON CACHE BOOL "Enable fanotify support")
        set(LIBFSWATCH_HEADER_FILES
                ${LIBFSWATCH_HEADER_FILES}
                src/libfswatch/c++/fanotify_monitor.hpp)
        set(LIB_SOURCE_FILES
                ${LIB_SOURCE_FILES}
                src/libfswatch/c++/fanotify_monitor.cpp)
    endif ()
endif ()

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

target_include_directories(libfswatch PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/libfswatch>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/libfswatch>
        ${Intl_INCLUDE_DIRS})
target_include_directories(libfswatch PRIVATE ${PROJECT_BINARY_DIR})
target_include_directories(libfswatch PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(libfswatch PRIVATE ${EXTRA_LIBS})

set(LIBFSWATCH_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/libfswatch")

install(TARGETS libfswatch
    EXPORT libfswatchTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
# TODO: should migrate to target_source(file_set) to install headers
install(DIRECTORY src/libfswatch
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING
    PATTERN "*.hpp"
    PATTERN "*.h"
    PATTERN ".deps" EXCLUDE
    PATTERN ".libs" EXCLUDE)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libfswatch_config.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libfswatch")

configure_package_config_file(
    cmake/libfswatchConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/libfswatchConfig.cmake"
    INSTALL_DESTINATION "${LIBFSWATCH_INSTALL_CMAKEDIR}")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/libfswatchConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion)
install(EXPORT libfswatchTargets
    NAMESPACE libfswatch::
    DESTINATION "${LIBFSWATCH_INSTALL_CMAKEDIR}")
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/libfswatchConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/libfswatchConfigVersion.cmake"
    DESTINATION "${LIBFSWATCH_INSTALL_CMAKEDIR}")
