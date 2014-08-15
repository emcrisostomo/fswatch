#
# Copyright (C) 2014, Enrico M. Crisostomo
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
ACLOCAL_AMFLAGS = -I m4

# Refresh configuration if libtool dependencies change
LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

lib_LTLIBRARIES = libfswatch.la

libfswatch_la_SOURCES  = c/libfswatch.cpp
libfswatch_la_SOURCES += c/libfswatch_log.cpp c/libfswatch_log.h
libfswatch_la_SOURCES += c++/libfswatch_map.h c++/libfswatch_set.h
libfswatch_la_SOURCES += c++/libfswatch_exception.cpp
libfswatch_la_SOURCES += c++/event.cpp
libfswatch_la_SOURCES += c++/monitor.cpp
libfswatch_la_SOURCES += c++/poll_monitor.cpp
if USE_CORESERVICES
  libfswatch_la_SOURCES += c++/fsevent_monitor.cpp
endif
if USE_KQUEUE
  libfswatch_la_SOURCES += c++/kqueue_monitor.cpp
endif
if USE_INOTIFY
  libfswatch_la_SOURCES += c++/inotify_monitor.cpp
endif
libfswatch_la_SOURCES += c++/path_utils.cpp c++/path_utils.h

libfswatch_la_LDFLAGS =

if USE_CORESERVICES
  libfswatch_la_LDFLAGS += -framework CoreServices
endif

# Define separate include directories for C and C++ headers.
libfswatch_cdir = $(pkgincludedir)/c
libfswatch_cppdir = $(pkgincludedir)/c++

# Distribute C headers.
libfswatch_c_HEADERS = c/libfswatch.h c/cevent.h c/cfilter.h c/cmonitor.h c/error.h

# Distribute C++ headers conditionally adding available backends.
libfswatch_cpp_HEADERS  = c++/monitor.h
if USE_CORESERVICES
  libfswatch_cpp_HEADERS += c++/fsevent_monitor.h 
endif
if USE_KQUEUE
  libfswatch_cpp_HEADERS += c++/kqueue_monitor.h
endif
if USE_INOTIFY
  libfswatch_cpp_HEADERS += c++/inotify_monitor.h
endif
libfswatch_cpp_HEADERS += c++/poll_monitor.h
libfswatch_cpp_HEADERS += c++/filter.h c++/event.h c++/libfswatch_exception.h
