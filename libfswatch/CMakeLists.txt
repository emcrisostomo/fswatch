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
include_directories(.. src/libfswatch)

set(LIB_SOURCE_FILES
        src/libfswatch/c/cevent.cpp
        src/libfswatch/c/cevent.h
        src/libfswatch/c/cfilter.h
        src/libfswatch/c/cmonitor.h
        src/libfswatch/c/error.h
        src/libfswatch/c/libfswatch.cpp
        src/libfswatch/c/libfswatch.h
        src/libfswatch/c/libfswatch_log.cpp
        src/libfswatch/c/libfswatch_log.h
        src/libfswatch/c/libfswatch_types.h
        src/libfswatch/c++/event.cpp
        src/libfswatch/c++/event.hpp
        src/libfswatch/c++/filter.hpp
        src/libfswatch/c++/filter.cpp
        src/libfswatch/c++/libfswatch_exception.cpp
        src/libfswatch/c++/libfswatch_exception.hpp
        src/libfswatch/c++/libfswatch_map.hpp
        src/libfswatch/c++/libfswatch_set.hpp
        src/libfswatch/c++/monitor.cpp
        src/libfswatch/c++/monitor.hpp
        src/libfswatch/c++/monitor_factory.cpp
        src/libfswatch/c++/monitor_factory.hpp
        src/libfswatch/c++/path_utils.cpp
        src/libfswatch/c++/path_utils.hpp
        src/libfswatch/c++/poll_monitor.cpp
        src/libfswatch/c++/poll_monitor.hpp
        src/libfswatch/c++/string/string_utils.cpp
        src/libfswatch/c++/string/string_utils.hpp
        src/libfswatch/gettext.h
        src/libfswatch/gettext_defs.h)

set(LIB_COMPILE_DEFINITIONS "")

if (EXISTS ../libfswatch_config.h)
    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            ../libfswatch_config.h)

    set(LIB_COMPILE_DEFINITIONS
            ${LIB_COMPILE_DEFINITIONS}
            -DHAVE_CONFIG_H)
endif ()

INCLUDE(CheckIncludeFiles)

CHECK_INCLUDE_FILES(sys/inotify.h HAVE_SYS_INOTIFY)

if (HAVE_SYS_INOTIFY_H)
    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            src/libfswatch/c++/inotify_monitor.cpp
            src/libfswatch/c++/inotify_monitor.hpp)
endif (HAVE_SYS_INOTIFY_H)

CHECK_INCLUDE_FILES(sys/event.h HAVE_SYS_EVENT_H)

if (HAVE_SYS_EVENT_H)
    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            src/libfswatch/c++/kqueue_monitor.cpp
            src/libfswatch/c++/kqueue_monitor.hpp)
endif (HAVE_SYS_EVENT_H)

CHECK_INCLUDE_FILES(port.h HAVE_PORT_H)

if (HAVE_PORT_H)
    set(LIB_SOURCE_FILES
            ${LIB_SOURCE_FILES}
            src/libfswatch/c++/fen_monitor.cpp
            src/libfswatch/c++/fen_monitor.hpp)
endif (HAVE_PORT_H)

if (WIN32)
    CHECK_INCLUDE_FILES(sys/cygwin.h HAVE_CYGWIN)

    set(LIB_COMPILE_DEFINITIONS
            ${LIB_COMPILE_DEFINITIONS}
            -DHAVE_STRUCT_STAT_ST_MTIME)

    if (HAVE_CYGWIN)
        set(LIB_SOURCE_FILES
                ${LIB_SOURCE_FILES}
                src/libfswatch/c++/windows/win_directory_change_event.cpp
                src/libfswatch/c++/windows/win_directory_change_event.hpp
                src/libfswatch/c++/windows/win_error_message.cpp
                src/libfswatch/c++/windows/win_error_message.hpp
                src/libfswatch/c++/windows/win_handle.cpp
                src/libfswatch/c++/windows/win_handle.hpp
                src/libfswatch/c++/windows/win_paths.cpp
                src/libfswatch/c++/windows/win_paths.hpp
                src/libfswatch/c++/windows/win_strings.cpp
                src/libfswatch/c++/windows/win_strings.hpp
                src/libfswatch/c++/windows_monitor.cpp
                src/libfswatch/c++/windows_monitor.hpp)
        add_definitions(-DHAVE_WINDOWS)
    endif (HAVE_CYGWIN)
elseif (APPLE)
    CHECK_INCLUDE_FILES(CoreServices/CoreServices.h HAVE_FSEVENTS_FILE_EVENTS)

    set(LIB_COMPILE_DEFINITIONS
            ${LIB_COMPILE_DEFINITIONS}
            -DHAVE_STRUCT_STAT_ST_MTIME)

    if (HAVE_FSEVENTS_FILE_EVENTS)
        find_library(CORESERVICES_LIBRARY CoreServices)
        set(EXTRA_LIBS ${CORESERVICES_LIBRARY})

        set(LIB_SOURCE_FILES
                ${LIB_SOURCE_FILES}
                src/libfswatch/c++/fsevents_monitor.cpp
                src/libfswatch/c++/fsevents_monitor.hpp)

    endif (HAVE_FSEVENTS_FILE_EVENTS)

    set(LIB_COMPILE_DEFINITIONS -DHAVE_STRUCT_STAT_ST_MTIMESPEC)
else () # Other Unix I suppose
    set(LIB_COMPILE_DEFINITIONS
            ${LIB_COMPILE_DEFINITIONS}
            -HAVE_STRUCT_STAT_ST_MTIMESPEC)
endif ()

add_library(libfswatch ${LIB_SOURCE_FILES})
target_include_directories(libfswatch PUBLIC src)
target_link_libraries(libfswatch ${CORESERVICES_LIBRARY})
target_compile_definitions(libfswatch PRIVATE ${LIB_COMPILE_DEFINITIONS})
