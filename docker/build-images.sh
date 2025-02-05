#!/bin/zsh
# -*- coding: utf-8; tab-width: 2; indent-tabs-mode: nil; sh-basic-offset: 2; sh-indentation: 2; -*- vim:fenc=utf-8:et:sw=2:ts=2:sts=2
#
# Copyright (C) 2016 Enrico M. Crisostomo
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
setopt local_options
setopt local_traps
unsetopt glob_subst

SCRIPT_HOME=${0:h}

for i in ${SCRIPT_HOME}/*/
do
  TAG=${i:t}
  [[ ${TAG} =~ ^[a-zA-Z0-9._-]+(/[a-zA-Z0-9._-]+)*$ ]] ||
  {
    >&2 print -- Invalid tag ${TAG}. Skipping.
    continue
  }
  docker build -t fswatch:${TAG} ${i}
done
