/*
 * Copyright (c) 2014-2018 Enrico M. Crisostomo
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

namespace fsw
{
  monitor *monitor_factory::create_monitor(fsw_monitor_type type,
                                           std::vector<std::string> paths,
                                           FSW_EVENT_CALLBACK *callback,
                                           void *context)
  {
    switch (type)
    {
    case system_default_monitor_type:
      return create_default_monitor(paths, callback, context);

    default:
      auto c = creators_by_type().find(type);

      if (c == creators_by_type().end())
        throw libfsw_exception("Unsupported monitor.",
                               FSW_ERR_UNKNOWN_MONITOR_TYPE);
      return c->second(paths, callback, context);
    }
  }

  std::map<std::string, FSW_FN_MONITOR_CREATOR>& monitor_factory::creators_by_string()
  {
    static std::map<std::string, FSW_FN_MONITOR_CREATOR> creator_by_string_map;

    return creator_by_string_map;
  }

  std::map<fsw_monitor_type, FSW_FN_MONITOR_CREATOR>&
  monitor_factory::creators_by_type()
  {
    static std::map<fsw_monitor_type, FSW_FN_MONITOR_CREATOR> creator_by_type_map;

    return creator_by_type_map;
  }

  monitor *monitor_factory::create_monitor(const std::string& name,
                                           std::vector<std::string> paths,
                                           FSW_EVENT_CALLBACK *callback,
                                           void *context)
  {
    auto i = creators_by_string().find(name);

    if (i == creators_by_string().end())
      return nullptr;

    return i->second(std::move(paths), callback, context);
  }

  bool monitor_factory::exists_type(const std::string& name)
  {
    auto i = creators_by_string().find(name);

    return (i != creators_by_string().end());
  }

  bool monitor_factory::exists_type(const fsw_monitor_type& type)
  {
    auto i = creators_by_type().find(type);

    return (i != creators_by_type().end());
  }

  std::vector<std::string> monitor_factory::get_types()
  {
    std::vector<std::string> types;

    for (const auto& i : creators_by_string())
    {
      types.push_back(i.first);
    }

    return types;
  }
}