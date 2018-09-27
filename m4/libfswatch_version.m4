#
# Copyright (c) 2014-2018 Enrico M. Crisostomo
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

# Here are a set of rules to help you update your library version information:
#
#   * Start with version information of ‘0:0:0’ for each libtool library.
#
#   * Update the version information only immediately before a public
#     release of your software.  More frequent updates are
#     unnecessary, and only guarantee that the current interface
#     number gets larger faster.
#
#   * If the library source code has changed at all since the last
#     update, then increment revision (‘c:r:a’ becomes ‘c:r+1:a’).
#
#   * If any interfaces have been added, removed, or changed since the
#     last update, increment current, and set revision to 0.
#
#   * If any interfaces have been added since the last public release,
#     then increment age.
#
#   * If any interfaces have been removed or changed since the last
#     public release, then set age to 0.
#
# Libtool documentation, 7.3 Updating library version information
#
m4_define([LIBFSWATCH_VERSION], [1.13.0])
m4_define([LIBFSWATCH_API_VERSION], [11:0:0])
m4_define([LIBFSWATCH_REVISION], [1])
