#
# Copyright (c) 2014-2017 Enrico M. Crisostomo
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
ACLOCAL_AMFLAGS = -I m4

# Refresh configuration if libtool dependencies change
LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) $(top_builddir)/config.status libtool

if USE_NLS
  FSW_PO_SUBDIR = po
endif

SUBDIRS = libfswatch fswatch $(FSW_PO_SUBDIR)

dist_man_MANS  = man/fswatch.7
dist_doc_DATA  = README.bsd
dist_doc_DATA += README.codestyle
dist_doc_DATA += README.freebsd
dist_doc_DATA += README.gnu-build-system
dist_doc_DATA += README.illumos
dist_doc_DATA += README.md
dist_doc_DATA += README.libfswatch.md
dist_doc_DATA += README.linux
dist_doc_DATA += README.osx
dist_doc_DATA += README.smartos
dist_doc_DATA += README.solaris
dist_doc_DATA += README.windows
if USE_NLS
  dist_doc_DATA += ABOUT-NLS
endif
dist_doc_DATA += AUTHORS
dist_doc_DATA += CONTRIBUTING.md
dist_doc_DATA += COPYING
dist_doc_DATA += NEWS
dist_doc_DATA += AUTHORS.libfswatch
dist_doc_DATA += NEWS.libfswatch

dist-hook:
# The recipe cannot assume a file in $(distdir) is writable.
	chmod u+w $(distdir)/ChangeLog
	git log --oneline --decorate >> $(distdir)/ChangeLog

# Manually distributing config.rpath seems not to be required any longer with
# current Autotools.
#
# EXTRA_DIST = config/config.rpath

docker-images:
	./docker/build-images.sh
