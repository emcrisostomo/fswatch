/* 
 * Copyright (C) 2014, Enrico M. Crisostomo
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
#ifndef FSW_H
#  define FSW_H

#  include <exception>
#  include <string>

#  define FSW_EXIT_OK      0
#  define FSW_EXIT_UNK_OPT 1
#  define FSW_EXIT_USAGE   2
#  define FSW_EXIT_LATENCY 4
#  define FSW_EXIT_STREAM  8
#  define FSW_EXIT_ERROR  16
#  define FSW_EXIT_ENFILE 32
#  define FSW_EXIT_OPT    64

bool is_verbose();

#endif  /* FSW_H */
