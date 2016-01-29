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
 * @brief Header defining the default set type used by the library.
 *
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */

#ifndef LIBFSW_SET_H
#  define LIBFSW_SET_H

#  ifdef HAVE_CONFIG_H
#    include "libfswatch_config.h"
#  endif

#  if defined(HAVE_UNORDERED_SET)
#    include <unordered_set>

namespace fsw
{
  /**
   * @brief Default set type used by `libfswatch`.
   *
   * This type definition will be a synonym of `std::unordered_set` if the C++
   * library contains it, otherwise it will default to `std::set`.
   */
  template<typename K>
  using fsw_hash_set = std::unordered_set<K>;
}

#  else
#    include <set>

namespace fsw
{
  /**
   * @brief Default set type used by `libfswatch`.
   *
   * This type definition will be a synonym of `std::unordered_set` if the C++
   * library contains it, otherwise it will default to `std::set`.
   */
  template <typename K>
  using fsw_hash_set = std::set<K>;
}

#  endif

#endif  /* LIBFSW_SET_H */
