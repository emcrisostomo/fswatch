/*
 * Copyright (C) 2014-2015 Enrico M. Crisostomo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FSW_GETTEXT_DEFS_H
#  define FSW_GETTEXT_DEFS_H

#include "gettext.h"

#  ifdef __cplusplus
extern "C"
{
#  endif

#define _(String) gettext(String)

#  ifdef __cplusplus
}
#  endif

#endif  /* FSW_GETTEXT_DEFS_H */
