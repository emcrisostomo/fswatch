#
# Copyright (c) 2014-2016 Enrico M. Crisostomo
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
add_definitions(-DHAVE_CONFIG_H)

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
        src/libfswatch/c++/libfswatch_map.hpp
        src/libfswatch/c++/libfswatch_set.hpp
        src/libfswatch/c++/monitor.hpp
        src/libfswatch/c++/path_utils.hpp
        src/libfswatch/c++/poll_monitor.hpp
        src/libfswatch/c++/string/string_utils.hpp
        src/libfswatch/gettext.h
        src/libfswatch/gettext_defs.h)

set(LIB_SOURCE_FILES
        src/libfswatch/c/cevent.cpp
        src/libfswatch/c/libfswatch.cpp
        src/libfswatch/c/libfswatch_log.cpp
        src/libfswatch/c++/event.cpp
        src/libfswatch/c++/filter.cpp
        src/libfswatch/c++/libfswatch_exception.cpp
        src/libfswatch/c++/monitor.cpp
        src/libfswatch/c++/path_utils.cpp
        src/libfswatch/c++/poll_monitor.cpp
        src/libfswatch/c++/string/string_utils.cpp)

include(CheckIncludeFiles)

check_include_files(sys/inotify.h HAVE_SYS_INOTIFY_H)

if (HAVE_SYS_INOTIFY_H)
    set(LIBFSWATCH_HEADER_FILES
            ${LIBFSWATCH_HEADER_FILES}
            src/libfswatch/c++/inotify_monitor.hpp)
    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            src/libfswatch/c++/inotify_monitor.cpp)
endif (HAVE_SYS_INOTIFY_H)

CHECK_INCLUDE_FILES(sys/event.h HAVE_SYS_EVENT_H)

if (HAVE_SYS_EVENT_H)
    set(LIBFSWATCH_HEADER_FILES
            ${LIBFSWATCH_HEADER_FILES}
            src/libfswatch/c++/kqueue_monitor.hpp)
    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            src/libfswatch/c++/kqueue_monitor.cpp)
endif (HAVE_SYS_EVENT_H)

CHECK_INCLUDE_FILES(port.h HAVE_PORT_H)

if (HAVE_PORT_H)
    set(LIBFSWATCH_HEADER_FILES
            ${LIBFSWATCH_HEADER_FILES}
            src/libfswatch/c++/fen_monitor.hpp)
    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            src/libfswatch/c++/fen_monitor.cpp)
endif (HAVE_PORT_H)

if (WIN32)
    CHECK_INCLUDE_FILES(sys/cygwin.h HAVE_CYGWIN)

    if (HAVE_CYGWIN)
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
        add_definitions(-DHAVE_WINDOWS)
    endif (HAVE_CYGWIN)
endif (WIN32)

if (APPLE)
    CHECK_INCLUDE_FILES(CoreServices/CoreServices.h HAVE_FSEVENTS_FILE_EVENTS)

    if (HAVE_FSEVENTS_FILE_EVENTS)
        find_library(CORESERVICES_LIBRARY CoreServices)
        set(EXTRA_LIBS ${CORESERVICES_LIBRARY})

        set(LIBFSWATCH_HEADER_FILES
                ${LIBFSWATCH_HEADER_FILES}
                src/libfswatch/c++/fsevents_monitor.hpp)
        set(LIB_SOURCE_FILES
                ${LIB_SOURCE_FILES}
                src/libfswatch/c++/fsevents_monitor.cpp)

    endif (HAVE_FSEVENTS_FILE_EVENTS)
endif (APPLE)

add_library(libfswatch ${LIB_SOURCE_FILES} ${LIBFSWATCH_HEADER_FILES})

# check for gettext and libintl
check_include_file_cxx(getopt.h HAVE_GETOPT_H)

if (HAVE_GETOPT_H)
    check_symbol_exists(getopt_long getopt.h HAVE_GETOPT_LONG)
endif (HAVE_GETOPT_H)

# If both gettext and libintl are found, define the USE_NLS variable to
# optionally disable them
if (Intl_FOUND AND GETTEXT_FOUND)
    option(USE_NLS "Use NLS" ON)
endif ()

if (USE_NLS)
    set(ENABLE_NLS 1)

    if (Intl_LIBRARIES)
        target_link_libraries(libfswatch ${Intl_LIBRARIES})
    endif ()

    # gettext configuration
    set(LINGUAS en en@quot en@boldquot it es)

    foreach (language ${LINGUAS})
        configure_file(${PROJECT_SOURCE_DIR}/po/${language}.po ${PROJECT_BINARY_DIR}/${language}.po COPYONLY)
    endforeach ()

    gettext_process_pot_file(po/${PACKAGE}.pot ALL INSTALL_DESTINATION share/locale LANGUAGES ${LINGUAS})
endif ()

target_include_directories(libfswatch PUBLIC .. src/libfswatch)
target_include_directories(libfswatch PUBLIC src)
target_include_directories(libfswatch PUBLIC ${Intl_INCLUDE_DIRS})
target_include_directories(libfswatch PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(libfswatch ${EXTRA_LIBS})

install(TARGETS libfswatch DESTINATION lib)
install(FILES ${LIBFSWATCH_HEADER_FILES} DESTINATION include/libfswatch)
