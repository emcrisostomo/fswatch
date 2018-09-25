/*
 * Copyright (c) 2014-2017 Enrico M. Crisostomo
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
#include "gettext.h"
#include "fswatch.hpp"
#include <iostream>
#include <string>
#include <exception>
#include <csignal>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cerrno>
#include <vector>
#include <map>
#include "libfswatch/c++/event.hpp"
#include "libfswatch/c++/monitor.hpp"
#include "libfswatch/c++/monitor_factory.hpp"
#include "libfswatch/c/error.h"
#include "libfswatch/c/libfswatch.h"
#include "libfswatch/c/libfswatch_log.h"
#include "libfswatch/c++/libfswatch_exception.hpp"

#ifdef HAVE_GETOPT_LONG
#  include <getopt.h>
#endif

#define _(String) gettext(String)

using namespace fsw;

/*
 * Event formatting types and routines.
 */
static void print_event_flags(const event& evt);
static void print_event_path(const event& evt);
static void print_event_timestamp(const event& evt);
static int printf_event_validate_format(const std::string& fmt);

static FSW_EVENT_CALLBACK process_events;

struct printf_event_callbacks
{
  void (*format_f)(const event& evt);
  void (*format_p)(const event& evt);
  void (*format_t)(const event& evt);
};

struct printf_event_callbacks event_format_callbacks
  {
    print_event_flags,
    print_event_path,
    print_event_timestamp
  };

static int printf_event(const std::string& fmt,
                        const event& evt,
                        const struct printf_event_callbacks& callback,
                        std::ostream& os = std::cout);

static const unsigned int TIME_FORMAT_BUFF_SIZE = 128;

static monitor *active_monitor = nullptr;
static std::vector<monitor_filter> filters;
static std::vector<fsw_event_type_filter> event_filters;
static std::vector<std::string> filter_files;
static bool _0flag = false;
static bool _1flag = false;
static bool aflag = false;
static bool allow_overflow = false;
static int batch_marker_flag = false;
static bool dflag = false;
static bool Eflag = false;
static bool fieFlag = false;
static bool Iflag = false;
static bool Lflag = false;
static bool mflag = false;
static bool nflag = false;
static bool oflag = false;
static bool rflag = false;
static bool tflag = false;
static bool uflag = false;
static bool vflag = false;
static int version_flag = false;
static bool xflag = false;
static double lvalue = 1.0;
static std::string monitor_name;
static std::string tformat = "%c";
static std::string batch_marker = event::get_event_flag_name(fsw_event_flag::NoOp);
static int format_flag = false;
static std::string format;
static std::string event_flag_separator = " ";
static std::map<std::string, std::string> monitor_properties;

/*
 * OPT_* variables are used as getopt_long values for long options that do not
 * have a short option equivalent.
 */
static const int OPT_BATCH_MARKER = 128;
static const int OPT_FORMAT = 129;
static const int OPT_EVENT_FLAG_SEPARATOR = 130;
static const int OPT_EVENT_TYPE = 131;
static const int OPT_ALLOW_OVERFLOW = 132;
static const int OPT_MONITOR_PROPERTY = 133;
static const int OPT_FIRE_IDLE_EVENTS = 134;
static const int OPT_FILTER_FROM = 135;

static void list_monitor_types(std::ostream& stream)
{
  for (const auto& type : monitor_factory::get_types())
  {
    stream << "  " << type << "\n";
  }
}

static void print_version(std::ostream& stream)
{
  stream << PACKAGE_STRING << "\n";
  stream << "Copyright (C) 2013-2018 Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>.\n";
  stream << _("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
  stream << _("This is free software: you are free to change and redistribute it.\n");
  stream << _("There is NO WARRANTY, to the extent permitted by law.\n");
  stream << "\n";
  stream << _("Written by Enrico M. Crisostomo.");
  stream << std::endl;
}

static void usage(std::ostream& stream)
{
#ifdef HAVE_GETOPT_LONG
  stream << PACKAGE_STRING << "\n\n";
  stream << _("Usage:\n");
  stream << PACKAGE_NAME << _(" [OPTION] ... path ...\n");
  stream << "\n";
  stream << _("Options:\n");
  stream << " -0, --print0          " << _("Use the ASCII NUL character (0) as line separator.\n");
  stream << " -1, --one-event       " << _("Exit fswatch after the first set of events is received.\n");
  stream << "     --allow-overflow  " << _("Allow a monitor to overflow and report it as a change event.\n");
  stream << "     --batch-marker    " << _("Print a marker at the end of every batch.\n");
  stream << " -a, --access          " << _("Watch file accesses.\n");
  stream << " -d, --directories     " << _("Watch directories only.\n");
  stream << " -e, --exclude=REGEX   " << _("Exclude paths matching REGEX.\n");
  stream << " -E, --extended        " << _("Use extended regular expressions.\n");
  stream << "     --filter-from=FILE\n";
  stream << "                       " << _("Load filters from file.") << "\n";
  stream << "     --format=FORMAT   " << _("Use the specified record format.") << "\n";
  stream << " -f, --format-time     " << _("Print the event time using the specified format.\n");
  stream << "     --fire-idle-event " << _("Fire idle events.\n");
  stream << " -h, --help            " << _("Show this message.\n");
  stream << " -i, --include=REGEX   " << _("Include paths matching REGEX.\n");
  stream << " -I, --insensitive     " << _("Use case insensitive regular expressions.\n");
  stream << " -l, --latency=DOUBLE  " << _("Set the latency.\n");
  stream << " -L, --follow-links    " << _("Follow symbolic links.\n");
  stream << " -M, --list-monitors   " << _("List the available monitors.\n");
  stream << " -m, --monitor=NAME    " << _("Use the specified monitor.\n");
  stream << "     --monitor-property name=value\n";
  stream << "                       " << _("Define the specified property.\n");
  stream << " -n, --numeric         " << _("Print a numeric event mask.\n");
  stream << " -o, --one-per-batch   " << _("Print a single message with the number of change events.\n");
  stream << " -r, --recursive       " << _("Recurse subdirectories.\n");
  stream << " -t, --timestamp       " << _("Print the event timestamp.\n");
  stream << " -u, --utc-time        " << _("Print the event time as UTC time.\n");
  stream << " -x, --event-flags     " << _("Print the event flags.\n");
  stream << "     --event=TYPE      " << _("Filter the event by the specified type.\n");
  stream << "     --event-flag-separator=STRING\n";
  stream << "                       " << _("Print event flags using the specified separator.") << "\n";
  stream << " -v, --verbose         " << _("Print verbose output.\n");
  stream << "     --version         " << _("Print the version of ") << PACKAGE_NAME << _(" and exit.\n");
  stream << "\n";
#else
  std::string option_string = "[01adeEfhilLMmnortuvx]";

  stream << PACKAGE_STRING << "\n\n";
  stream << _("Usage:\n");
  stream << PACKAGE_NAME << " " << option_string << " path ...\n";
  stream << "\n";
  stream << _("Options:\n");
  stream << " -0  Use the ASCII NUL character (0) as line separator.\n";
  stream << " -1  Exit fswatch after the first set of events is received.\n";
  stream << " -a  Watch file accesses.\n";
  stream << " -d  Watch directories only.\n";
  stream << " -e  Exclude paths matching REGEX.\n";
  stream << " -E  Use extended regular expressions.\n";
  stream << " -f  Print the event time stamp with the specified format.\n";
  stream << " -h  Show this message.\n";
  stream << " -i  Include paths matching REGEX.\n";
  stream << " -I  Use case insensitive regular expressions.\n";
  stream << " -l  Set the latency.\n";
  stream << " -L  Follow symbolic links.\n";
  stream << " -m  Use the specified monitor.\n";
  stream << " -M  List the available monitors.\n";
  stream << " -n  Print a numeric event masks.\n";
  stream << " -o  Print a single message with the number of change events in the current\n";
  stream << "     batch.\n";
  stream << " -r  Recurse subdirectories.\n";
  stream << " -t  Print the event timestamp.\n";
  stream << " -u  Print the event time as UTC time.\n";
  stream << " -v  Print verbose output.\n";
  stream << " -x  Print the event flags.\n";
  stream << "\n";
#endif

  stream << _("Available monitors in this platform:\n\n");
  list_monitor_types(stream);

  stream << _("\nSee the man page for more information.\n\n");

  stream << _("Report bugs to <") << PACKAGE_BUGREPORT << ">.\n";
  stream << PACKAGE << _(" home page: <") << PACKAGE_URL << ">.";
  stream << std::endl;
}

static void close_monitor()
{
  if (active_monitor) active_monitor->stop();
}

namespace
{
  extern "C" void close_handler(int signal)
  {
    FSW_ELOG(_("Executing termination handler.\n"));
    close_monitor();
  }
}

static bool parse_event_bitmask(const char *optarg)
{
  try
  {
    auto bitmask = std::stoul(optarg, nullptr, 10);

    for (auto& item : FSW_ALL_EVENT_FLAGS)
    {
      if ((bitmask & item) == item)
      {
        event_filters.push_back({item});
      }
    }

    return true;
  }
  catch (std::invalid_argument& ex)
  {
    return false;
  }
}

static bool parse_event_filter(const char *optarg)
{
  if (parse_event_bitmask(optarg)) return true;

  try
  {
    event_filters.push_back({event::get_event_flag_by_name(optarg)});
    return true;
  }
  catch (libfsw_exception& ex)
  {
    std::cerr << ex.what() << std::endl;
    return false;
  }
}

static bool validate_latency(double latency)
{
  if (latency == 0.0)
  {
    std::cerr << _("Invalid value: ") << optarg << std::endl;
    return false;
  }

  if (errno == ERANGE || latency == HUGE_VAL)
  {
    std::cerr << _("Value out of range: ") << optarg << std::endl;
    return false;
  }

  return true;
}

static void register_signal_handlers()
{
  struct sigaction action;
  action.sa_handler = close_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  if (sigaction(SIGTERM, &action, nullptr) == 0)
  {
    FSW_ELOG(_("SIGTERM handler registered.\n"));
  }
  else
  {
    std::cerr << _("SIGTERM handler registration failed.") << std::endl;
  }

  if (sigaction(SIGABRT, &action, nullptr) == 0)
  {
    FSW_ELOG(_("SIGABRT handler registered.\n"));
  }
  else
  {
    std::cerr << _("SIGABRT handler registration failed.") << std::endl;
  }

  if (sigaction(SIGINT, &action, nullptr) == 0)
  {
    FSW_ELOG(_("SIGINT handler registered.\n"));
  }
  else
  {
    std::cerr << _("SIGINT handler registration failed") << std::endl;
  }
}

static void print_event_path(const event& evt)
{
  std::cout << evt.get_path();
}

static void print_event_timestamp(const event& evt)
{
  const time_t& evt_time = evt.get_time();

  char time_format_buffer[TIME_FORMAT_BUFF_SIZE];
  struct tm *tm_time = uflag ? gmtime(&evt_time) : localtime(&evt_time);

  std::string date =
    strftime(time_format_buffer,
             TIME_FORMAT_BUFF_SIZE,
             tformat.c_str(),
             tm_time) ? std::string(time_format_buffer) : std::string(
      _("<date format error>"));

  std::cout << date;
}

static void print_event_flags(const event& evt)
{
  const std::vector<fsw_event_flag>& flags = evt.get_flags();

  if (nflag)
  {
    int mask = 0;
    for (const fsw_event_flag& flag : flags)
    {
      mask += static_cast<int> (flag);
    }

    std::cout << mask;
  }
  else
  {
    for (size_t i = 0; i < flags.size(); ++i)
    {
      std::cout << flags[i];

      // Event flag separator is currently hard-coded.
      if (i != flags.size() - 1) std::cout << event_flag_separator;
    }
  }
}

static void print_end_of_event_record()
{
  if (_0flag)
  {
    std::cout << '\0';
    std::cout.flush();
  }
  else
  {
    std::cout << std::endl;
  }
}

static void write_batch_marker()
{
  if (batch_marker_flag)
  {
    std::cout << batch_marker;
    print_end_of_event_record();
  }
}

static void write_one_batch_event(const std::vector<event>& events)
{
  std::cout << events.size();
  print_end_of_event_record();

  write_batch_marker();
}

static void write_events(const std::vector<event>& events)
{
  for (const event& evt : events)
  {
    printf_event(format, evt, event_format_callbacks);
    print_end_of_event_record();
  }

  write_batch_marker();

  if (_1flag)
  {
    close_monitor();
  }
}

void process_events(const std::vector<event>& events, void *context)
{
  if (oflag)
    write_one_batch_event(events);
  else
    write_events(events);
}

static void start_monitor(int argc, char **argv, int optind)
{
  // parsing paths
  std::vector<std::string> paths;

  for (auto i = optind; i < argc; ++i)
  {
    char *real_path = realpath(argv[i], nullptr);
    std::string path(real_path ? real_path : argv[i]);

    if (real_path) free(real_path);

    FSW_ELOGF(_("Adding path: %s\n"), path.c_str());

    paths.push_back(path);
  }

  if (mflag)
    active_monitor = monitor_factory::create_monitor(monitor_name,
                                                     paths,
                                                     process_events);
  else
    active_monitor = monitor_factory::create_monitor(
      fsw_monitor_type::system_default_monitor_type,
      paths,
      process_events);

  /*
   * libfswatch supports case sensitivity and extended flags to be set on any
   * filter but fswatch does not.  For the time being, we apply the same flags
   * to every filter.
   */
  for (auto& filter : filters)
  {
    filter.case_sensitive = !Iflag;
    filter.extended = Eflag;
  }

  // Load filters from the specified files.
  for (const auto& filter_file : filter_files)
  {
    auto filters_from_file =
      monitor_filter::read_from_file(filter_file,
                                     [](std::string f)
                                     {
                                       std::cerr << _("Invalid filter: ") << f
                                            << "\n";
                                     });

    std::move(filters_from_file.begin(),
              filters_from_file.end(),
              std::back_inserter(filters));
  }

  active_monitor->set_properties(monitor_properties);
  active_monitor->set_allow_overflow(allow_overflow);
  active_monitor->set_latency(lvalue);
  active_monitor->set_fire_idle_event(fieFlag);
  active_monitor->set_recursive(rflag);
  active_monitor->set_directory_only(dflag);
  active_monitor->set_event_type_filters(event_filters);
  active_monitor->set_filters(filters);
  active_monitor->set_follow_symlinks(Lflag);
  active_monitor->set_watch_access(aflag);

  active_monitor->start();
}

static void parse_opts(int argc, char **argv)
{
  int ch;
  std::string short_options = "01ade:Ef:hi:Il:LMm:nortuvx";

#ifdef HAVE_GETOPT_LONG
  int option_index = 0;
  static struct option long_options[] = {
    {"access",               no_argument,       nullptr,       'a'},
    {"allow-overflow",       no_argument,       nullptr,       OPT_ALLOW_OVERFLOW},
    {"batch-marker",         optional_argument, nullptr,       OPT_BATCH_MARKER},
    {"directories",          no_argument,       nullptr,       'd'},
    {"event",                required_argument, nullptr,       OPT_EVENT_TYPE},
    {"event-flags",          no_argument,       nullptr,       'x'},
    {"event-flag-separator", required_argument, nullptr,       OPT_EVENT_FLAG_SEPARATOR},
    {"exclude",              required_argument, nullptr,       'e'},
    {"extended",             no_argument,       nullptr,       'E'},
    {"filter-from",          required_argument, nullptr,       OPT_FILTER_FROM},
    {"fire-idle-events",     no_argument,       nullptr,       OPT_FIRE_IDLE_EVENTS},
    {"follow-links",         no_argument,       nullptr,       'L'},
    {"format",               required_argument, nullptr,       OPT_FORMAT},
    {"format-time",          required_argument, nullptr,       'f'},
    {"help",                 no_argument,       nullptr,       'h'},
    {"include",              required_argument, nullptr,       'i'},
    {"insensitive",          no_argument,       nullptr,       'I'},
    {"latency",              required_argument, nullptr,       'l'},
    {"list-monitors",        no_argument,       nullptr,       'M'},
    {"monitor",              required_argument, nullptr,       'm'},
    {"monitor-property",     required_argument, nullptr,       OPT_MONITOR_PROPERTY},
    {"numeric",              no_argument,       nullptr,       'n'},
    {"one-per-batch",        no_argument,       nullptr,       'o'},
    {"one-event",            no_argument,       nullptr,       '1'},
    {"print0",               no_argument,       nullptr,       '0'},
    {"recursive",            no_argument,       nullptr,       'r'},
    {"timestamp",            no_argument,       nullptr,       't'},
    {"utc-time",             no_argument,       nullptr,       'u'},
    {"verbose",              no_argument,       nullptr,       'v'},
    {"version",              no_argument,       &version_flag, true},
    {nullptr, 0,                                nullptr,       0}
  };

  while ((ch = getopt_long(argc,
                           argv,
                           short_options.c_str(),
                           long_options,
                           &option_index)) != -1)
  {
#else
    while ((ch = getopt(argc, argv, short_options.c_str())) != -1)
    {
#endif

    switch (ch)
    {
    case '0':
      _0flag = true;
      break;

    case '1':
      _1flag = true;
      break;

    case 'a':
      aflag = true;
      break;

    case 'd':
      dflag = true;
      break;

    case 'e':
      filters.push_back({optarg, fsw_filter_type::filter_exclude});
      break;

    case 'E':
      Eflag = true;
      break;

    case 'f':
      tformat = std::string(optarg);
      break;

    case 'h':
      usage(std::cout);
      exit(FSW_EXIT_OK);

    case 'i':
      filters.push_back({optarg, fsw_filter_type::filter_include});
      break;

    case 'I':
      Iflag = true;
      break;

    case 'l':
      lvalue = strtod(optarg, nullptr);

      if (!validate_latency(lvalue))
      {
        exit(FSW_EXIT_LATENCY);
      }

      break;

    case 'L':
      Lflag = true;
      break;

    case 'M':
      list_monitor_types(std::cout);
      exit(FSW_EXIT_OK);

    case 'm':
      mflag = true;
      monitor_name = std::string(optarg);
      break;

    case 'n':
      nflag = true;
      xflag = true;
      break;

    case 'o':
      oflag = true;
      break;

    case 'r':
      rflag = true;
      break;

    case 't':
      tflag = true;
      break;

    case 'u':
      uflag = true;
      break;

    case 'v':
      vflag = true;
      break;

    case 'x':
      xflag = true;
      break;

    case OPT_BATCH_MARKER:
      if (optarg) batch_marker = optarg;
      batch_marker_flag = true;
      break;

    case OPT_FORMAT:
      format_flag = true;
      format = optarg;
      break;

    case OPT_EVENT_FLAG_SEPARATOR:
      event_flag_separator = optarg;
      break;

    case OPT_EVENT_TYPE:
      if (!parse_event_filter(optarg))
      {
        exit(FSW_ERR_UNKNOWN_VALUE);
      }
      break;

    case OPT_ALLOW_OVERFLOW:
      allow_overflow = true;
      break;

    case OPT_MONITOR_PROPERTY:
    {
      std::string param(optarg);
      size_t eq_pos = param.find_first_of('=');
      if (eq_pos == std::string::npos)
      {
        std::cerr << _("Invalid property format.") << std::endl;
        exit(FSW_ERR_INVALID_PROPERTY);
      }

      monitor_properties[param.substr(0, eq_pos)] = param.substr(eq_pos + 1);
    }
      break;

    case OPT_FIRE_IDLE_EVENTS:
      fieFlag = true;
      break;

    case OPT_FILTER_FROM:
      filter_files.emplace_back(optarg);
      break;

    case '?':
      usage(std::cerr);
      exit(FSW_EXIT_UNK_OPT);
    }
  }

  // Set verbose mode for libfswatch.
  fsw_set_verbose(vflag);

  if (version_flag)
  {
    print_version(std::cout);
    exit(FSW_EXIT_OK);
  }

  // --format is incompatible with any other format option.
  if (format_flag && (tflag || xflag))
  {
    std::cerr <<
         _("--format is incompatible with any other format option such as -t and -x.")
         <<
         std::endl;
    exit(FSW_EXIT_FORMAT);
  }

  if (format_flag && oflag)
  {
    std::cerr << _("--format is incompatible with -o.") << std::endl;
    exit(FSW_EXIT_FORMAT);
  }

  // If no format was specified use:
  //   * %p as the default.
  //   * -t adds "%t " at the beginning of the format.
  //   * -x adds " %f" at the end of the format.
  //   * '\n' is used as record separator unless -0 is used, in which case '\0'
  //     is used instead.
  if (format_flag)
  {
    // Test the user format
    if (printf_event_validate_format(format) < 0)
    {
      std::cerr << _("Invalid format.") << std::endl;
      exit(FSW_EXIT_FORMAT);
    }
  }
  else
  {
    // Build event format.
    if (tflag)
    {
      format = "%t ";
    }

    format += "%p";

    if (xflag)
    {
      format += " %f";
    }
  }
}

static void format_noop(const event& evt)
{
}

static int printf_event_validate_format(const std::string& fmt)
{

  struct printf_event_callbacks noop_callbacks
    {
      format_noop,
      format_noop,
      format_noop
    };

  const std::vector<fsw_event_flag> flags;
  const event empty("", 0, flags);
  std::ostream noop_stream(nullptr);

  return printf_event(fmt, empty, noop_callbacks, noop_stream);
}

static int printf_event(const std::string& fmt,
                        const event& evt,
                        const struct printf_event_callbacks& callback,
                        std::ostream& os)
{
  /*
   * %t - time (further formatted using -f and strftime.
   * %p - event path
   * %f - event flags (event separator will be formatted with a separate option)
   */
  for (size_t i = 0; i < format.length(); ++i)
  {
    // If the character does not start a format directive, copy it as it is.
    if (format[i] != '%')
    {
      os << format[i];
      continue;
    }

    // If this is the end of the string, dump an error.
    if (i == format.length() - 1)
    {
      return -1;
    }

    // Advance to next format and check which directive it is.
    const char c = format[++i];

    switch (c)
    {
    case '%':
      os << '%';
      break;
    case '0':
      os << '\0';
      break;
    case 'n':
      os << '\n';
      break;
    case 'f':
      callback.format_f(evt);
      break;
    case 'p':
      callback.format_p(evt);
      break;
    case 't':
      callback.format_t(evt);
      break;
    default:
      return -1;
    }
  }

  return 0;
}

int main(int argc, char **argv)
{
  // Trigger gettext operations
#ifdef ENABLE_NLS
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif

  parse_opts(argc, argv);

  // validate options
  if (optind == argc)
  {
    std::cerr << _("Invalid number of arguments.") << std::endl;
    exit(FSW_EXIT_UNK_OPT);
  }

  if (mflag && !monitor_factory::exists_type(monitor_name))
  {
    std::cerr << _("Invalid monitor name.") << std::endl;
    exit(FSW_EXIT_MONITOR_NAME);
  }

  // configure and start the monitor
  try
  {
    // registering handlers to clean up resources
    register_signal_handlers();
    atexit(close_monitor);

    // configure and start the monitor loop
    start_monitor(argc, argv, optind);

    delete active_monitor;
    active_monitor = nullptr;
  }
  catch (libfsw_exception& lex)
  {
    std::cerr << lex.what() << "\n";
    std::cerr << "Status code: " << lex.error_code() << "\n";

    return FSW_EXIT_ERROR;
  }
  catch (std::invalid_argument& ex)
  {
    std::cerr << ex.what() << "\n";

    return FSW_EXIT_ERROR;
  }
  catch (std::exception& conf)
  {
    std::cerr << _("An error occurred and the program will be terminated.\n");
    std::cerr << conf.what() << "\n";

    return FSW_EXIT_ERROR;
  }
  catch (...)
  {
    std::cerr << _("An unknown error occurred and the program will be terminated.\n");

    return FSW_EXIT_ERROR;
  }

  return FSW_EXIT_OK;
}
