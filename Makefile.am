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
bin_PROGRAMS = fswatch
fswatch_SOURCES  = fswatch.cpp fswatch.h fswatch_log.cpp fswatch_log.h
fswatch_SOURCES += path_utils.cpp path_utils.h
fswatch_SOURCES += fswatch_exception.cpp fswatch_exception.h event.cpp event.h
fswatch_SOURCES += monitor.cpp monitor.h fswatch_map.h fswatch_set.h
fswatch_SOURCES += fsevent_monitor.cpp fsevent_monitor.h
fswatch_SOURCES += kqueue_monitor.cpp kqueue_monitor.h
fswatch_SOURCES += poll_monitor.cpp poll_monitor.h
fswatch_SOURCES += inotify_monitor.cpp inotify_monitor.h

if USE_CORESERVICES
  fswatch_LDFLAGS = -framework CoreServices
endif

man_MANS = fswatch.7
EXTRA_DIST = $(man_MANS)
dist_doc_DATA  = README.bsd README.freebsd README.gnu-build-system README.md
dist_doc_DATA += README.osx
