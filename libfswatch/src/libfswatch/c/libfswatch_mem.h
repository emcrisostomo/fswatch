/*
 * Copyright (c) 2015 Enrico M. Crisostomo
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 3, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBFSWATCH_MEM_H
#  define	LIBFSWATCH_MEM_H
#include <cstddef>

#  ifdef	__cplusplus
extern "C"
{
#  endif

  void * fsw_alloc(size_t size);
  void fsw_free(void * ptr);

#  ifdef	__cplusplus
}
#  endif

#endif	/* LIBFSWATCH_MEM_H */

