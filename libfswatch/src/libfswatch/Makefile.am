#
# Copyright (c) 2014-2015 Enrico M. Crisostomo
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
LIBFSWATCH_API_VERSION = @AM_LIBFSWATCH_API_VERSION@

# Prepare gettext-related symbols used by programs
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"

lib_LTLIBRARIES = libfswatch.la

libfswatch_la_SOURCES  = c/libfswatch_mem.cpp
libfswatch_la_SOURCES += c/cevent.cpp
libfswatch_la_SOURCES += c/libfswatch.cpp
libfswatch_la_SOURCES += c/libfswatch_log.cpp
libfswatch_la_SOURCES += c++/libfswatch_exception.cpp
libfswatch_la_SOURCES += c++/event.cpp
libfswatch_la_SOURCES += c++/monitor.cpp
libfswatch_la_SOURCES += c++/poll_monitor.cpp
libfswatch_la_SOURCES += c++/path_utils.cpp
libfswatch_la_SOURCES += gettext.h
libfswatch_la_SOURCES += gettext_defs.h

# Platform-dependent source files
if USE_FSEVENTS
  libfswatch_la_SOURCES += c++/fsevents_monitor.cpp
endif
if USE_KQUEUE
  libfswatch_la_SOURCES += c++/kqueue_monitor.cpp
endif
if USE_INOTIFY
  libfswatch_la_SOURCES += c++/inotify_monitor.cpp
endif

libfswatch_la_LDFLAGS = -version-info $(LIBFSWATCH_API_VERSION)

if USE_FSEVENTS
  libfswatch_la_LDFLAGS += -framework CoreServices
endif

# Define separate include directories for C and C++ headers.
libfswatch_cdir = $(pkgincludedir)/c
libfswatch_cppdir = $(pkgincludedir)/c++

# Distribute C headers.
libfswatch_c_HEADERS  = c/libfswatch_mem.h
libfswatch_c_HEADERS += c/libfswatch.h
libfswatch_c_HEADERS += c/cevent.h
libfswatch_c_HEADERS += c/cfilter.h
libfswatch_c_HEADERS += c/cmonitor.h
libfswatch_c_HEADERS += c/error.h
libfswatch_c_HEADERS += c/libfswatch_log.h

# Distribute C++ headers conditionally adding available backends.
libfswatch_cpp_HEADERS  = c++/monitor.hpp
libfswatch_cpp_HEADERS += c++/libfswatch_map.hpp
libfswatch_cpp_HEADERS += c++/libfswatch_set.hpp
libfswatch_cpp_HEADERS += c++/path_utils.hpp
if USE_FSEVENTS
  libfswatch_cpp_HEADERS += c++/fsevents_monitor.hpp
endif
if USE_KQUEUE
  libfswatch_cpp_HEADERS += c++/kqueue_monitor.hpp
endif
if USE_INOTIFY
  libfswatch_cpp_HEADERS += c++/inotify_monitor.hpp
endif
libfswatch_cpp_HEADERS += c++/poll_monitor.hpp
libfswatch_cpp_HEADERS += c++/filter.hpp
libfswatch_cpp_HEADERS += c++/event.hpp
libfswatch_cpp_HEADERS += c++/libfswatch_exception.hpp
