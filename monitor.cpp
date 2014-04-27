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

void monitor::set_exclude(const vector<string> &exclusions,
                          bool case_sensitive,
                          bool extended)
{
#ifdef HAVE_REGCOMP
  for (string exclusion : exclusions)
  {
    regex_t regex;
    int flags = 0;

    if (!case_sensitive) flags |= REG_ICASE;
    if (extended) flags |= REG_EXTENDED;

    if (::regcomp(&regex, exclusion.c_str(), flags))
    {
      string err = "An error occurred during the compilation of " + exclusion;
      throw fsw_exception(err);
    }

    exclude_regex.push_back(regex);
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
  for (auto re : exclude_regex)
  {
    if (::regexec(&re, path, 0, nullptr, 0) == 0)
    {
      return false;
    }
  }
#endif

  return true;
}

monitor::~monitor()
{
#ifdef HAVE_REGCOMP
  for (auto &re : exclude_regex)
  {
    ::regfree(&re);
  }

  exclude_regex.clear();
#endif
}
