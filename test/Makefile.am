#
# Copyright (c) 2026 Enrico M. Crisostomo
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

AM_CPPFLAGS = -I$(top_srcdir)/libfswatch/src -I$(top_builddir)
LDADD = $(top_builddir)/libfswatch/src/libfswatch.la

LOG_COMPILER = $(SHELL)
AM_TESTS_ENVIRONMENT = \
  FSWATCH='$(abs_top_builddir)/fswatch/src/fswatch'; export FSWATCH; \
  GIT='git'; export GIT;

TESTS =
EXTRA_DIST =
check_PROGRAMS =

check_PROGRAMS += filter_mode_test
filter_mode_test_SOURCES = src/filter_mode_test.cpp
TESTS += filter_mode_test

TESTS += poll_filter_root_path.sh
TESTS += poll_filter_mode.sh
TESTS += poll_prune.sh

if USE_INOTIFY
  check_PROGRAMS += inotify_stop_latency_test
  check_PROGRAMS += inotify_stop_with_pending_events_test
  check_PROGRAMS += inotify_stop_with_ready_events_test

  inotify_stop_latency_test_SOURCES = src/inotify_stop_latency_test.cpp
  inotify_stop_with_pending_events_test_SOURCES = src/inotify_stop_with_pending_events_test.cpp
  inotify_stop_with_ready_events_test_SOURCES = src/inotify_stop_with_ready_events_test.cpp

  TESTS += inotify_basic_events.sh
  TESTS += inotify_access_events.sh
  TESTS += inotify_missing_root_rescan.sh
  TESTS += inotify_watched_file_recreate.sh
  TESTS += inotify_watched_directory_replace.sh
  TESTS += inotify_burst_create.sh
  TESTS += inotify_queue_drain.sh
  TESTS += inotify_recursive_create.sh
  TESTS += inotify_filter_root_path.sh
  TESTS += inotify_filter_mode.sh
  TESTS += inotify_prune.sh
  TESTS += inotify_stop_latency_test
  TESTS += inotify_stop_with_pending_events_test
  TESTS += inotify_stop_with_ready_events_test
endif

if USE_FANOTIFY
  TESTS += fanotify_basic.sh
  TESTS += fanotify_access_events.sh
  TESTS += fanotify_unlimited_properties.sh
  TESTS += fanotify_filter_root_path.sh
  TESTS += fanotify_filter_mode.sh
  TESTS += fanotify_prune.sh
endif

if USE_FSEVENTS
  TESTS += fsevents_filter_root_path.sh
  TESTS += fsevents_filter_mode.sh
endif

EXTRA_DIST += inotify_basic_events.sh
EXTRA_DIST += inotify_access_events.sh
EXTRA_DIST += inotify_missing_root_rescan.sh
EXTRA_DIST += inotify_watched_file_recreate.sh
EXTRA_DIST += inotify_watched_directory_replace.sh
EXTRA_DIST += inotify_burst_create.sh
EXTRA_DIST += inotify_queue_drain.sh
EXTRA_DIST += inotify_recursive_create.sh
EXTRA_DIST += inotify_filter_root_path.sh
EXTRA_DIST += inotify_filter_mode.sh
EXTRA_DIST += inotify_prune.sh
EXTRA_DIST += poll_filter_root_path.sh
EXTRA_DIST += poll_filter_mode.sh
EXTRA_DIST += poll_prune.sh
EXTRA_DIST += filter_root_path.sh
EXTRA_DIST += filter_mode.sh
EXTRA_DIST += prune.sh
EXTRA_DIST += fanotify_basic.sh
EXTRA_DIST += fanotify_access_events.sh
EXTRA_DIST += fanotify_unlimited_properties.sh
EXTRA_DIST += fanotify_filter_root_path.sh
EXTRA_DIST += fanotify_filter_mode.sh
EXTRA_DIST += fanotify_prune.sh
EXTRA_DIST += fsevents_filter_root_path.sh
EXTRA_DIST += fsevents_filter_mode.sh
