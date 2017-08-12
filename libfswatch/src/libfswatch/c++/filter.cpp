/*
 * Copyright (c) 2016 Enrico M. Crisostomo
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

#include "gettext_defs.h"
#include "filter.hpp"
#include <fstream>
#include <regex>
#include <stdexcept>

using namespace std;

namespace fsw
{

  static inline bool parse_filter(std::string filter,
                                  monitor_filter& filter_object,
                                  void (*err_handler)(std::string));
  static inline bool is_unescaped_space(string& filter, long i);

  vector<monitor_filter> monitor_filter::read_from_file(const string& path,
                                                        void (*err_handler)(
                                                          string))
  {
    vector<monitor_filter> filters;
    ifstream f(path);

    if (!f.is_open())
      throw invalid_argument(string(_("File not found: ")) + path);

    string line;
    monitor_filter filter;

    while (getline(f, line))
    {
      if (parse_filter(line, filter, err_handler))
      {
        filters.push_back(filter);
      }
    }

    return filters;
  }

  bool is_unescaped_space(string& filter, long i)
  {
    if (filter[i] != ' ') return false;

    unsigned int backslashes = 0;

    while (--i >= 0 && filter[i] == '\\') ++backslashes;

    return (backslashes % 2 == 0);
  }

  bool parse_filter(string filter,
                    monitor_filter& filter_object,
                    void (*err_handler)(string))
  {
#define handle_error(t) if (err_handler) err_handler(t);
    // Skip empty strings.
    if (filter.length() == 0) return false;

    // Strip comments.
    if (filter[0] == '#') return false;

    // Valid filters have the following structure:
    //     type pattern
    // where type may contains the following characters:
    //   - '+' or '-', to indicate whether the filter is an inclusion or an exclusion filter.
    //   - 'e', for an extended regular expression.
    //   - 'i', for a case insensitive regular expression.
    regex filter_grammar("^([+-])([ei]*) (.+)$", regex_constants::extended);
    smatch fragments;

    if (!regex_match(filter, fragments, filter_grammar))
    {
      handle_error(filter);
      return false;
    }

    // Reset the filter object to its default values.
    filter_object = {};
    filter_object.case_sensitive = true;

    // Name the fragments
    string frag_type = fragments[1].str();
    string frag_flag = fragments[2].str();
    string frag_filter = fragments[3].str();

    // Build the filter
    switch (frag_type[0])
    {
    case '+':
      filter_object.type = fsw_filter_type::filter_include;
      break;
    case '-':
      filter_object.type = fsw_filter_type::filter_exclude;
      break;
    default:
      throw invalid_argument(string(_("Unknown filter type: ")) + frag_type[0]);
    }

    // Parse the flags
    for (char c : frag_flag)
    {
      switch (c)
      {
      case 'e':
        filter_object.extended = true;
        break;
      case 'i':
        filter_object.case_sensitive = false;
        break;
      default:
        throw invalid_argument(string(_("Unknown flag: ")) + c);
      }
    }

    // Parse the filter
    // Trim unescaped trailing spaces.
    for (auto i = frag_filter.length() - 1; i > 0; --i)
    {
      if (is_unescaped_space(frag_filter, i)) frag_filter.erase(i, 1);
      else break;
    }

    // If a single space is the only character left then the filter is invalid.
    if (frag_filter.length() == 1 && frag_filter[0] == ' ')
    {
      handle_error(filter);
      return false;
    }

    // Ignore empty lines.
    if (frag_filter.length() == 0)
    {
      handle_error(filter);
      return false;
    }

    // Copy filter to output.
    filter_object.text = frag_filter;

    return true;
  }
}
