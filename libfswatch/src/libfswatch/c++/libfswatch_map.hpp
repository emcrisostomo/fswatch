/*
 * Copyright (c) 2014-2015 Enrico M. Crisostomo
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
/**
 * @file
 * @brief Header defining the associative container used by the library.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef LIBFSW_MAP_H
#  define LIBFSW_MAP_H

#  ifdef HAVE_CONFIG_H
#    include "libfswatch_config.h"
#  endif

#  if defined(HAVE_UNORDERED_MAP)
#    include <unordered_map>

namespace fsw
{
  /**
   * @brief Default associative container type used by `libfswatch`.
   *
   * This type definition will be a synonym of `std::unordered_map` if the C++
   * library contains it, otherwise it will default to `std::map`.
   */
  template<typename K, typename V>
  using fsw_hash_map = std::unordered_map<K, V>;
}

#  else
#    include <map>

namespace fsw
{
  /**
   * @brief Default associative container type used by `libfswatch`.
   *
   * This type definition will be a synonym of `std::unordered_map` if the C++
   * library contains it, otherwise it will default to `std::map`.
   */
  template <typename K, typename V>
  using fsw_hash_map = std::map<K, V>;
}

#  endif

#endif  /* LIBFSW_MAP_H */
