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

LOG_COMPILER = $(SHELL)
AM_TESTS_ENVIRONMENT = \
  FSWATCH='$(abs_top_builddir)/fswatch/src/fswatch'; export FSWATCH; \
  GIT='git'; export GIT;

TESTS =

if USE_INOTIFY
TESTS += inotify_access_events.sh
TESTS += inotify_recursive_create.sh
endif

if USE_FANOTIFY
TESTS += fanotify_basic.sh
TESTS += fanotify_access_events.sh
endif

EXTRA_DIST = \
  STRESS_TESTS.md \
  inotify_access_events.sh \
  inotify_recursive_create.sh \
  fanotify_basic.sh \
  fanotify_access_events.sh
