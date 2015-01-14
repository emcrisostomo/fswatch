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
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "gettext.h"
#include "fswatch.h"
#include "fswatch_log.h"
#include <iostream>
#include <sstream>
#include <csignal>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cerrno>
#include <vector>
#include "libfswatch/c++/monitor.h"

#ifdef HAVE_GETOPT_LONG
#  include <getopt.h>
#endif

#define _(String) gettext(String)

using namespace std;

static string decode_event_flag_name(fsw_event_flag flag);

/*
 * Event formatting types and routines.
 */
static void print_event_flags(const event & evt);
static void print_event_path(const event & evt);
static void print_event_timestamp(const event & evt);

static int printf_event_validate_format(const string & fmt);

struct printf_event_callbacks
{
  void (*format_f)(const event & evt);
  void (*format_p)(const event & evt);
  void (*format_t)(const event & evt);
};

struct printf_event_callbacks event_format_callbacks
{
  print_event_flags,
  print_event_path,
  print_event_timestamp
};

static int printf_event(const string & fmt,
                        const event & evt,
                        const struct printf_event_callbacks & callback,
                        ostream & os = cout);

static const unsigned int TIME_FORMAT_BUFF_SIZE = 128;

static fsw::monitor *active_monitor = nullptr;
static vector<monitor_filter> filters;
static bool _0flag = false;
static bool _1flag = false;
static int batch_marker_flag = false;
static bool Eflag = false;
static bool fflag = false;
static bool Iflag = false;
static bool lflag = false;
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
static string monitor_name;
static string tformat = "%c";
static string batch_marker = decode_event_flag_name(fsw_event_flag::NoOp);
static int format_flag = false;
static string format;
static string event_flag_separator = " ";

/*
 * OPT_* variables are used as getopt_long values for long options that do not
 * have a short option equivalent.
 */
static const int OPT_BATCH_MARKER = 128;
static const int OPT_FORMAT = 129;
static const int OPT_EVENT_FLAG_SEPARATOR = 130;

bool is_verbose()
{
  return vflag;
}

static void list_monitor_types(ostream& stream)
{
  stream << _("Available monitors in this platform:\n\n");

  for (const auto & type : fsw::monitor_factory::get_types())
  {
    stream << "  " << type << "\n";
  }
}

static void print_version(ostream& stream)
{
  stream << PACKAGE_STRING << "\n";
  stream << "Copyright (C) 2014, 2015, Enrico M. Crisostomo <enrico.m.crisostomo@gmail.com>.\n";
  stream << _("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
  stream << _("This is free software: you are free to change and redistribute it.\n");
  stream << _("There is NO WARRANTY, to the extent permitted by law.\n");
  stream << "\n";
  stream << _("Written by Enrico M. Crisostomo.");
  stream << endl;
}

static void usage(ostream& stream)
{
#ifdef HAVE_GETOPT_LONG
  stream << PACKAGE_STRING << "\n\n";
  stream << _("Usage:\n");
  stream << PACKAGE_NAME << _(" [OPTION] ... path ...\n");
  stream << "\n";
  stream << _("Options:\n");
  stream << " -0, --print0          " << _("Use the ASCII NUL character (0) as line separator.\n");
  stream << " -1, --one-event       " << _("Exit fswatch after the first set of events is received.\n");
  stream << "     --batch-marker    " << _("Print a marker at the end of every batch.\n");
#  ifdef HAVE_REGCOMP
  stream << " -e, --exclude=REGEX   " << _("Exclude paths matching REGEX.\n");
  stream << " -E, --extended        " << _("Use extended regular expressions.\n");
#  endif
  stream << "     --format=FORMAT   " << _("Use the specified record format.") << "\n";
  stream << " -f, --format-time     " << _("Print the event time using the specified format.\n");
  stream << " -h, --help            " << _("Show this message.\n");
#  ifdef HAVE_REGCOMP
  stream << " -i, --include=REGEX   " << _("Include paths matching REGEX.\n");
  stream << " -I, --insensitive     " << _("Use case insensitive regular expressions.\n");
#  endif
  stream << " -l, --latency=DOUBLE  " << _("Set the latency.\n");
  stream << " -L, --follow-links    " << _("Follow symbolic links.\n");
  stream << " -m, --monitor=NAME    " << _("Use the specified monitor.\n");
  stream << " -n, --numeric         " << _("Print a numeric event mask.\n");
  stream << " -o, --one-per-batch   " << _("Print a single message with the number of change events.\n");
  stream << " -r, --recursive       " << _("Recurse subdirectories.\n");
  stream << " -t, --timestamp       " << _("Print the event timestamp.\n");
  stream << " -u, --utc-time        " << _("Print the event time as UTC time.\n");
  stream << " -v, --verbose         " << _("Print verbose output.\n");
  stream << "     --version         " << _("Print the version of ") << PACKAGE_NAME << _(" and exit.\n");
  stream << " -x, --event-flags     " << _("Print the event flags.\n");
  stream << "     --event-flag-separator=STRING\n";
  stream << "                       " << _("Print event flags using the specified separator.") << "\n";
  stream << "\n";
#else
  string option_string = "[";
  option_string += "01";
#  ifdef HAVE_REGCOMP
  option_string += "eE";
#  endif
  option_string += "fh";
#  ifdef HAVE_REGCOMP
  option_string += "i";
#  endif
  option_string += "lLmnortuvx";
  option_string += "]";

  stream << PACKAGE_STRING << "\n\n";
  stream << "Syntax:\n";
  stream << PACKAGE_NAME << " " << option_string << " path ...\n";
  stream << "\n";
  stream << "Usage:\n";
  stream << " -0  Use the ASCII NUL character (0) as line separator.\n";
  stream << " -1  Exit fswatch after the first set of events is received.\n";
#  ifdef HAVE_REGCOMP
  stream << " -e  Exclude paths matching REGEX.\n";
  stream << " -E  Use extended regular expressions.\n";
#  endif
  stream << " -f  Print the event time stamp with the specified format.\n";
  stream << " -h  Show this message.\n";
#  ifdef HAVE_REGCOMP
  stream << " -i  Use case insensitive regular expressions.\n";
  stream << " -i  Include paths matching REGEX.\n";
#  endif
  stream << " -l  Set the latency.\n";
  stream << " -m  Use the specified monitor.\n";
  stream << " -L  Follow symbolic links.\n";
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

  list_monitor_types(stream);

  stream << _("\nSee the man page for more information.\n\n");

  stream << _("Report bugs to <") << PACKAGE_BUGREPORT << ">.\n";
  stream << PACKAGE << _(" home page: <") << PACKAGE_URL << ">.";
  stream << endl;
}

static void close_stream()
{
  if (active_monitor)
  {
    delete active_monitor;

    active_monitor = nullptr;
  }
}

static void close_handler(int signal)
{
  close_stream();

  fsw_log(_("Done.\n"));
  exit(FSW_EXIT_OK);
}

static bool validate_latency(double latency, ostream &ost, ostream &est)
{
  if (lvalue == 0.0)
  {
    est << _("Invalid value: ") << optarg << endl;
    return false;
  }

  if (errno == ERANGE || lvalue == HUGE_VAL)
  {
    est << _("Value out of range: ") << optarg << endl;
    return false;
  }

  if (is_verbose())
  {
    ost << _("Latency set to: ") << lvalue << endl;
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
    fsw_log(_("SIGTERM handler registered.\n"));
  }
  else
  {
    cerr << _("SIGTERM handler registration failed.") << endl;
  }

  if (sigaction(SIGABRT, &action, nullptr) == 0)
  {
    fsw_log(_("SIGABRT handler registered.\n"));
  }
  else
  {
    cerr << _("SIGABRT handler registration failed.") << endl;
  }

  if (sigaction(SIGINT, &action, nullptr) == 0)
  {
    fsw_log(_("SIGINT handler registered.\n"));
  }
  else
  {
    cerr << _("SIGINT handler registration failed") << endl;
  }
}

static string decode_event_flag_name(fsw_event_flag flag)
{
  switch (flag)
  {
  case fsw_event_flag::NoOp:
    return "NoOp";
  case fsw_event_flag::PlatformSpecific:
    return "PlatformSpecific";
  case fsw_event_flag::Created:
    return "Created";
  case fsw_event_flag::Updated:
    return "Updated";
  case fsw_event_flag::Removed:
    return "Removed";
  case fsw_event_flag::Renamed:
    return "Renamed";
  case fsw_event_flag::OwnerModified:
    return "OwnerModified";
  case fsw_event_flag::AttributeModified:
    return "AttributeModified";
  case fsw_event_flag::MovedFrom:
    return "MovedFrom";
  case fsw_event_flag::MovedTo:
    return "MovedTo";
  case fsw_event_flag::IsFile:
    return "IsFile";
  case fsw_event_flag::IsDir:
    return "IsDir";
  case fsw_event_flag::IsSymLink:
    return "IsSymLink";
  case fsw_event_flag::Link:
    return "Link";
  default:
    return "<Unknown>";
  }
}

static vector<string> decode_event_flag_names(vector<fsw_event_flag> flags)
{
  vector<string> names;

  for (fsw_event_flag flag : flags)
  {
    names.push_back(decode_event_flag_name(flag));
  }

  return names;
}

static void print_event_path(const event & evt)
{
  cout << evt.get_path();
}

static void print_event_timestamp(const event & evt)
{
  const time_t & evt_time = evt.get_time();

  char time_format_buffer[TIME_FORMAT_BUFF_SIZE];
  struct tm * tm_time = uflag ? gmtime(&evt_time) : localtime(&evt_time);

  string date =
    strftime(time_format_buffer,
             TIME_FORMAT_BUFF_SIZE,
             tformat.c_str(),
             tm_time) ? string(time_format_buffer) : string(_("<date format error>"));

  cout << date;
}

static void print_event_flags(const event & evt)
{
  const vector<fsw_event_flag> & flags = evt.get_flags();

  if (nflag)
  {
    int mask = 0;
    for (const fsw_event_flag &flag : flags)
    {
      mask += static_cast<int> (flag);
    }

    cout << mask;
  }
  else
  {
    vector<string> flag_names = decode_event_flag_names(flags);

    for (int i = 0; i < flag_names.size(); ++i)
    {
      const string &name = flag_names[i];
      cout << name;

      // Event flag separator is currently hard-coded.
      if (i != flag_names.size() - 1) cout << event_flag_separator;
    }
  }
}

static void print_end_of_event_record()
{
  if (_0flag)
  {
    cout << '\0';
    cout.flush();
  }
  else
  {
    cout << endl;
  }
}

static void write_batch_marker()
{
  if (batch_marker_flag)
  {
    cout << batch_marker;
    print_end_of_event_record();
  }
}

static void write_one_batch_event(const vector<event> &events)
{
  cout << events.size();
  print_end_of_event_record();

  write_batch_marker();
}

static void write_events(const vector<event> &events)
{
  for (const event &evt : events)
  {
    printf_event(format, evt, event_format_callbacks);
    print_end_of_event_record();
  }

  write_batch_marker();

  if (_1flag)
  {
    ::exit(FSW_EXIT_OK);
  }
}

static void process_events(const vector<event> &events, void * context)
{
  if (oflag)
    write_one_batch_event(events);
  else
    write_events(events);
}

static void start_monitor(int argc, char ** argv, int optind)
{
  // parsing paths
  vector<string> paths;

  for (auto i = optind; i < argc; ++i)
  {
    char *real_path = ::realpath(argv[i], nullptr);
    string path(real_path ? real_path : argv[i]);

    if (real_path)
    {
      ::free(real_path);
    }

    fsw_log(_("Adding path: "));
    fsw_log(path.c_str());
    fsw_log("\n");

    paths.push_back(path);
  }

  if (mflag)
    active_monitor = fsw::monitor_factory::create_monitor_by_name(monitor_name,
                                                                  paths,
                                                                  process_events);
  else
    active_monitor = fsw::monitor::create_default_monitor(paths,
                                                          process_events);

  /* 
   * libfswatch supports case sensitivity and extended flags to be set on any
   * filter but fswatch does not.  For the time being, we apply the same flags
   * to every filter.
   */
  for (auto & filter : filters)
  {
    filter.case_sensitive = !Iflag;
    filter.extended = Eflag;
  }

  active_monitor->set_latency(lvalue);
  active_monitor->set_recursive(rflag);
  active_monitor->set_filters(filters);
  active_monitor->set_follow_symlinks(Lflag);

  active_monitor->start();
}

static void parse_opts(int argc, char ** argv)
{
  int ch;
  ostringstream short_options;

  short_options << "01f:hl:Lm:nortuvx";
#ifdef HAVE_REGCOMP
  short_options << "e:Ei:I";
#endif
  short_options << "k";

#ifdef HAVE_GETOPT_LONG
  int option_index = 0;
  static struct option long_options[] = {
    { "print0", no_argument, nullptr, '0'},
    { "one-event", no_argument, nullptr, '1'},
    { "batch-marker", optional_argument, nullptr, OPT_BATCH_MARKER},
#  ifdef HAVE_REGCOMP
    { "exclude", required_argument, nullptr, 'e'},
    { "extended", no_argument, nullptr, 'E'},
#  endif
    { "format", required_argument, nullptr, OPT_FORMAT},
    { "format-time", required_argument, nullptr, 'f'},
    { "help", no_argument, nullptr, 'h'},
#  ifdef HAVE_REGCOMP
    { "include", required_argument, nullptr, 'i'},
    { "insensitive", no_argument, nullptr, 'I'},
#  endif
    { "latency", required_argument, nullptr, 'l'},
    { "follow-links", no_argument, nullptr, 'L'},
    { "monitor", required_argument, nullptr, 'm'},
    { "numeric", no_argument, nullptr, 'n'},
    { "one-per-batch", no_argument, nullptr, 'o'},
    { "recursive", no_argument, nullptr, 'r'},
    { "timestamp", no_argument, nullptr, 't'},
    { "utc-time", no_argument, nullptr, 'u'},
    { "verbose", no_argument, nullptr, 'v'},
    { "version", no_argument, &version_flag, true},
    { "event-flags", no_argument, nullptr, 'x'},
    { "event-flag-separator", required_argument, nullptr, OPT_EVENT_FLAG_SEPARATOR},
    { nullptr, 0, nullptr, 0}
  };

  while ((ch = getopt_long(argc,
                           argv,
                           short_options.str().c_str(),
                           long_options,
                           &option_index)) != -1)
  {
#else
  while ((ch = getopt(argc, argv, short_options.str().c_str())) != -1)
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

#ifdef HAVE_REGCOMP
    case 'e':
      filters.push_back({optarg, fsw_filter_type::filter_exclude});
      break;

    case 'E':
      Eflag = true;
      break;
#endif

    case 'f':
      fflag = true;
      tformat = string(optarg);
      break;

    case 'h':
      usage(cout);
      ::exit(FSW_EXIT_OK);

#ifdef HAVE_REGCOMP
    case 'i':
      filters.push_back({optarg, fsw_filter_type::filter_include});
      break;

    case 'I':
      Iflag = true;
      break;
#endif

    case 'l':
      lflag = true;
      lvalue = strtod(optarg, nullptr);

      if (!validate_latency(lvalue, cout, cerr))
      {
        exit(FSW_EXIT_LATENCY);
      }

      break;

    case 'L':
      Lflag = true;
      break;

    case 'm':
      mflag = true;
      monitor_name = string(optarg);
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
      
    case '?':
      usage(cerr);
      exit(FSW_EXIT_UNK_OPT);
    }
  }

  if (version_flag)
  {
    print_version(cout);
    ::exit(FSW_EXIT_OK);
  }

  // --format is incompatible with any other format option.
  if (format_flag && (tflag || xflag))
  {
    cerr << _("--format is incompatible with any other format option such as -t and -x.") << endl;
    ::exit(FSW_EXIT_FORMAT);
  }

  if (format_flag && oflag)
  {
    cerr << _("--format is incompatible with -o.") << endl;
    ::exit(FSW_EXIT_FORMAT);
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
      cerr << _("Invalid format.") << endl;
      ::exit(FSW_EXIT_FORMAT);
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

static void format_noop(const event & evt)
{
}

static int printf_event_validate_format(const string & fmt)
{

  struct printf_event_callbacks noop_callbacks
  {
    format_noop,
    format_noop,
    format_noop
  };

  const vector<fsw_event_flag> flags;
  const event empty("", 0, flags);
  ostream noop_stream(nullptr);

  return printf_event(fmt, empty, noop_callbacks, noop_stream);
}

static int printf_event(const string & fmt,
                        const event & evt,
                        const struct printf_event_callbacks & callback,
                        ostream & os)
{
  /*
   * %t - time (further formatted using -f and strftime.
   * %p - event path
   * %f - event flags (event separator will be formatted with a separate option)
   */
  for (auto i = 0; i < format.length(); ++i)
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

int main(int argc, char ** argv)
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
    cerr << _("Invalid number of arguments.") << endl;
    ::exit(FSW_EXIT_UNK_OPT);
  }

  if (mflag && !fsw::monitor_factory::exists_type(monitor_name))
  {
    cerr << _("Invalid monitor name.") << endl;
    ::exit(FSW_EXIT_MONITOR_NAME);
  }

  // configure and start the monitor
  try
  {
    // registering handlers to clean up resources
    register_signal_handlers();
    ::atexit(close_stream);

    // configure and start the monitor loop
    start_monitor(argc, argv, optind);
  }
  catch (exception & conf)
  {
    cerr << _("An error occurred and the program will be terminated.\n");
    cerr << conf.what() << endl;

    return FSW_EXIT_ERROR;
  }
  catch (...)
  {
    cerr << _("An unknown error occurred and the program will be terminated.") << endl;

    return FSW_EXIT_ERROR;
  }

  return FSW_EXIT_OK;
}
