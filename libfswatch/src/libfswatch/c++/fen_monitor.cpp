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
#ifdef HAVE_CONFIG_H
#  include "libfswatch_config.h"
#endif

#ifdef HAVE_PORT_H

#  include "gettext_defs.h"
#  include "fen_monitor.hpp"
#  include "libfswatch_map.hpp"
#  include "libfswatch_set.hpp"
#  include "libfswatch_exception.hpp"
#  include "../c/libfswatch_log.h"
#  include "path_utils.hpp"

using namespace std;

namespace fsw
{

  struct fen_monitor_load
  {
  };

  typedef struct FenFlagType
  {
    uint32_t flag;
    fsw_event_flag type;
  } FenFlagType;

  static vector<FenFlagType> create_flag_type_vector()
  {
    vector<FenFlagType> flags;
    // flags.push_back({NOTE_DELETE, fsw_event_flag::Removed});

    return flags;
  }

  static const vector<FenFlagType> event_flag_type = create_flag_type_vector();

  REGISTER_MONITOR_IMPL(fen_monitor, fen_monitor_type);

  fen_monitor::fen_monitor(vector<string> paths_to_monitor,
                           FSW_EVENT_CALLBACK * callback,
                           void * context) :
    monitor(paths_to_monitor, callback, context), load(new fen_monitor_load())
  {
  }

  fen_monitor::~fen_monitor()
  {
    delete load;
  }

  static vector<fsw_event_flag> decode_flags(uint32_t flag)
  {
    vector<fsw_event_flag> evt_flags;

    for (const FenFlagType &type : event_flag_type)
    {
      if (flag & type.flag)
      {
        evt_flags.push_back(type.type);
      }
    }

    return evt_flags;
  }


  void fen_monitor::run()
  {
  }
}

#endif  /* HAVE_PORT_H */
