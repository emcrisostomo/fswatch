#
# Copyright (c) 2014-2024 Enrico M. Crisostomo
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

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libfswatch.pc

LIBFSWATCH_API_VERSION = @AM_LIBFSWATCH_API_VERSION@

# Prepare gettext-related symbols used by programs
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -I$(top_builddir)

lib_LTLIBRARIES = libfswatch.la

libfswatch_la_SOURCES  = libfswatch/c/cevent.cpp
libfswatch_la_SOURCES += libfswatch/c/libfswatch.cpp
libfswatch_la_SOURCES += libfswatch/c/libfswatch_log.cpp
libfswatch_la_SOURCES += libfswatch/c++/libfswatch_exception.cpp
libfswatch_la_SOURCES += libfswatch/c++/event.cpp
libfswatch_la_SOURCES += libfswatch/c++/filter.cpp
libfswatch_la_SOURCES += libfswatch/c++/monitor.cpp
libfswatch_la_SOURCES += libfswatch/c++/monitor_factory.cpp
libfswatch_la_SOURCES += libfswatch/c++/poll_monitor.cpp
libfswatch_la_SOURCES += libfswatch/c++/path_utils.cpp
libfswatch_la_SOURCES += libfswatch/c++/string/string_utils.cpp
libfswatch_la_SOURCES += libfswatch/gettext.h
libfswatch_la_SOURCES += libfswatch/gettext_defs.h

# Platform-dependent source files
if USE_FSEVENTS
  libfswatch_la_SOURCES += libfswatch/c++/fsevents_monitor.cpp
endif
if USE_KQUEUE
  libfswatch_la_SOURCES += libfswatch/c++/kqueue_monitor.cpp
endif
if USE_FEN
  libfswatch_la_SOURCES += libfswatch/c++/fen_monitor.cpp
endif
if USE_INOTIFY
  libfswatch_la_SOURCES += libfswatch/c++/inotify_monitor.cpp
endif
if USE_WINDOWS
if USE_CYGWIN
  libfswatch_la_SOURCES += libfswatch/c++/windows_monitor.cpp
  libfswatch_la_SOURCES += libfswatch/c++/windows/win_handle.cpp
  libfswatch_la_SOURCES += libfswatch/c++/windows/win_handle.hpp
  libfswatch_la_SOURCES += libfswatch/c++/windows/win_error_message.cpp
  libfswatch_la_SOURCES += libfswatch/c++/windows/win_error_message.hpp
  libfswatch_la_SOURCES += libfswatch/c++/windows/win_strings.cpp
  libfswatch_la_SOURCES += libfswatch/c++/windows/win_strings.hpp
  libfswatch_la_SOURCES += libfswatch/c++/windows/win_paths.cpp
  libfswatch_la_SOURCES += libfswatch/c++/windows/win_paths.hpp
  libfswatch_la_SOURCES += libfswatch/c++/windows/win_directory_change_event.cpp
  libfswatch_la_SOURCES += libfswatch/c++/windows/win_directory_change_event.hpp
endif
endif

libfswatch_la_LDFLAGS  = -version-info $(LIBFSWATCH_API_VERSION)
libfswatch_la_LDFLAGS += @LTLIBINTL@

if OS_CYGWIN
  libfswatch_la_LDFLAGS += -no-undefined
endif

if USE_FSEVENTS
  libfswatch_la_LDFLAGS += -framework CoreServices
endif

# Define separate include directories for C and C++ headers.
libfswatch_cdir = $(includedir)/libfswatch/c
libfswatch_cppdir = $(includedir)/libfswatch/c++

# Distribute C headers.
libfswatch_c_HEADERS  = libfswatch/c/libfswatch.h
libfswatch_c_HEADERS += libfswatch/c/libfswatch_types.h
libfswatch_c_HEADERS += libfswatch/c/cevent.h
libfswatch_c_HEADERS += libfswatch/c/cfilter.h
libfswatch_c_HEADERS += libfswatch/c/cmonitor.h
libfswatch_c_HEADERS += libfswatch/c/error.h
libfswatch_c_HEADERS += libfswatch/c/libfswatch_log.h

# Distribute C++ headers conditionally adding available backends.
libfswatch_cpp_HEADERS  = libfswatch/c++/monitor.hpp
libfswatch_cpp_HEADERS += libfswatch/c++/monitor_factory.hpp
libfswatch_cpp_HEADERS += libfswatch/c++/path_utils.hpp
libfswatch_cpp_HEADERS += libfswatch/c++/string/string_utils.hpp
if USE_FSEVENTS
  libfswatch_cpp_HEADERS += libfswatch/c++/fsevents_monitor.hpp
endif
if USE_KQUEUE
  libfswatch_cpp_HEADERS += libfswatch/c++/kqueue_monitor.hpp
endif
if USE_FEN
  libfswatch_cpp_HEADERS += libfswatch/c++/fen_monitor.hpp
endif
if USE_INOTIFY
  libfswatch_cpp_HEADERS += libfswatch/c++/inotify_monitor.hpp
endif
if USE_WINDOWS
  libfswatch_cpp_HEADERS += libfswatch/c++/windows_monitor.hpp
endif
libfswatch_cpp_HEADERS += libfswatch/c++/poll_monitor.hpp
libfswatch_cpp_HEADERS += libfswatch/c++/filter.hpp
libfswatch_cpp_HEADERS += libfswatch/c++/event.hpp
libfswatch_cpp_HEADERS += libfswatch/c++/libfswatch_exception.hpp
