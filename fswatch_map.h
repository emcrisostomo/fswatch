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
#ifndef FSW_MAP_H
#define FSW_MAP_H

#include "config.h"

#if defined(HAVE_UNORDERED_MAP)
#include <unordered_map>

template <typename K, typename V>
using fsw_hash_map = std::unordered_map<K, V>;

#else
#include <map>

template <typename K, typename V>
using fsw_hash_map = std::map<K, V>;

#endif

#endif  /* FSW_MAP_H */
