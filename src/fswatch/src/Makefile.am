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
AUTOMAKE_OPTIONS = std-options
bin_PROGRAMS = fswatch
fswatch_SOURCES  = fswatch.hpp fswatch.cpp
fswatch_SOURCES += gettext.h

# Set include path for libfswatch
AM_CPPFLAGS  = -I$(top_srcdir)/libfswatch/src
# Prepare gettext-related symbols used by programs
AM_CPPFLAGS += -DLOCALEDIR=\"$(localedir)\"

# Link fswatch against dependent libraries
fswatch_LDADD  = $(top_builddir)/libfswatch/src/libfswatch/libfswatch.la
# Link fswatch against libintl if gettext is being used
fswatch_LDADD += @LTLIBINTL@
