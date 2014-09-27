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
	$(SHELL) $(top_builddir)/config.status libtool

SUBDIRS = libfswatch doc src po

dist_man_MANS = man/fswatch.7
dist_doc_DATA  = README.bsd README.freebsd README.gnu-build-system README.md
dist_doc_DATA += README.linux README.osx AUTHORS COPYING LICENSE NEWS

dist_bin_SCRIPTS = 
if HAVE_ZSH
  dist_bin_SCRIPTS += scripts/fswatch-run-zsh
endif
if HAVE_BASH
  dist_bin_SCRIPTS += scripts/fswatch-run-bash
endif

install-exec-hook: fswatch-remove-links
if HAVE_ZSH
	$(LN_S) $(bindir)/fswatch-run-zsh $(DESTDIR)$(bindir)/fswatch-run
else
if HAVE_BASH
	$(LN_S) $(bindir)/fswatch-run-bash $(DESTDIR)$(bindir)/fswatch-run
else
	@echo "No compatible shell was found for fswatch-run."
endif
endif

uninstall-local: fswatch-remove-links

fswatch-remove-links:
	-rm $(DESTDIR)$(bindir)/fswatch-run

dist-hook:
# The recipe cannot assume a file in $(distdir) is writable.
	chmod u+w $(distdir)/ChangeLog
	git log --oneline --decorate >> $(distdir)/ChangeLog

EXTRA_DIST = config/config.rpath
