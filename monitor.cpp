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
#include "config.h"
#include "monitor.h"
#include "fswatch_exception.h"
#include <cstdlib>

using namespace std;

struct compiled_monitor_filter
{
  regex_t regex;
  filter_type type;
};

monitor::monitor(vector<string> paths_to_watch, EVENT_CALLBACK callback) :
  paths(paths_to_watch), callback(callback)
{
  if (callback == nullptr)
  {
    throw fsw_exception("Callback cannot be null.");
  }
}

void monitor::set_latency(double latency)
{
  if (latency < 0)
  {
    throw fsw_exception("Latency cannot be negative.");
  }

  this->latency = latency;
}

void monitor::set_recursive(bool recursive)
{
  this->recursive = recursive;
}

void monitor::set_filters(const std::vector<monitor_filter> &filters,
                          bool case_sensitive,
                          bool extended)
{
#ifdef HAVE_REGCOMP
  for (const monitor_filter &filter : filters)
  {
    regex_t regex;
    int flags = 0;

    if (!case_sensitive) flags |= REG_ICASE;
    if (extended) flags |= REG_EXTENDED;

    if (::regcomp(&regex, filter.text.c_str(), flags))
    {
      string err = "An error occurred during the compilation of " + filter.text;
      throw fsw_exception(err);
    }

    this->filters.push_back({regex, filter.type});
  }
#endif  
}

void monitor::set_follow_symlinks(bool follow)
{
  follow_symlinks = follow;
}

bool monitor::accept_path(const string &path)
{
  return accept_path(path.c_str());
}

bool monitor::accept_path(const char *path)
{
#ifdef HAVE_REGCOMP
  for (auto &filter : filters)
  {
    if (::regexec(&filter.regex, path, 0, nullptr, 0) == 0)
    {
      return filter.type == filter_type::filter_include;
    }
  }
#endif

  return true;
}

monitor::~monitor()
{
#ifdef HAVE_REGCOMP
  for (auto &re : filters)
  {
    ::regfree(&re.regex);
  }

  filters.clear();
#endif
}
