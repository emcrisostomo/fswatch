/*
 * Copyright (c) 2014-2016 Enrico M. Crisostomo
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
 * @brief Main `libfswatch` source file.
 * @copyright Copyright (c) 2014-2016 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.10.0
 */
/**
 * @mainpage
 *
 * @section introduction Introduction
 *
 * `fswatch` is a cross-platform file change monitor currently supporting the
 * following backends:
 *
 *   - A monitor based on the _FSEvents_ API of Apple OS X.
 *   - A monitor based on _kqueue_, an event notification interface introduced
 *     in FreeBSD 4.1 and supported on most *BSD systems (including OS X).
 *   - A monitor based on _File Events Notification_, an event notification API
 *     of the Solaris/Illumos kernel.
 *   - A monitor based on _inotify_, a Linux kernel subsystem that reports
 *     file system changes to applications.
 *   - A monitor based on the Microsoft Windows' `ReadDirectoryChangesW`
 *     function and reads change events asynchronously.
 *   - A monitor which periodically stats the file system, saves file
 *     modification times in memory and manually calculates file system
 *     changes, which can work on any operating system where stat can be
 *     used.
 *
 * Instead of using different APIs, a programmer can use just one: the API of
 * `libfswatch`.  The advantages of using `libfswatch` are many:
 *
 *   - *Portability*: `libfswatch` supports many backends, effectively giving
 *     support to a great number of operating systems, including Solaris, *BSD
 *     Unix and Linux.
 *   - Ease of use: using `libfswatch` should be easier than using any of the
 *     APIs it supports.
 *
 * @section changelog Changelog
 *
 * See the @ref history "History" page.
 *
 * @section bindings Available Bindings
 *
 * `libfswatch` is a C++ library with C bindings which makes it available to a
 * wide range of programming languages.  If a programming language has C
 * bindings, then `libfswatch` can be used from it.  The C binding provides all
 * the functionality provided by the C++ implementation and it can be used as a
 * fallback solution when the C++ API cannot be used.
 *
 * @section libtools-versioning libtool's versioning scheme
 *
 * `libtool`'s versioning scheme is described by three integers:
 * `current:revision:age` where:
 *
 *   - `current` is the most recent interface number implemented by the
 *     library.
 *   - `revision` is the implementation number of the current interface.
 *   - `age` is the difference between the newest and the oldest interface that
 *     the library implements.
 *
 * @section c-cpp-api The C and the C++ API
 *
 * The C API is built on top of the C++ API but the two are very different, to
 * reflect the fundamental differences between the two languages.
 *
 * The \ref cpp-api "C++ API" centres on the concept of _monitor_, a class of
 * objects modelling the functionality of the file monitoring API.  Different
 * monitor types are modelled as different classes inheriting from the
 * `fsw::monitor` abstract class, that is the type that defines the core
 * monitoring API.  API clients can pick the current platform's default monitor,
 * or choose a specific implementation amongst the available ones, configure it
 * and _run_ it.  When running, a monitor gathers file system change events and
 * communicates them back to the caller using a _callback_.
 *
 * The \ref c-api "C API", on the other hand, centres on the concept of
 * _monitoring session_.  A session internally wraps a monitor instance and
 * represents an opaque C bridge to the C++ monitor _API_.  Sessions are
 * identified by a _session handle_ and they can be thought as a sort of C
 * facade of the C++ monitor class.  In fact there is an evident similarity
 * between the C library functions operating on a monitoring session and the
 * methods of the `monitor` class.
 *
 * @section thread-safety Thread Safety
 *
 * The C++ API does not deal with thread safety explicitly.  Rather, it leaves
 * the responsibility of implementing a thread-safe use of the library to the
 * callers.  The C++ implementation has been designed in order to:
 *
 *   - Encapsulate all the state of a monitor into its class fields.
 *   - Perform no concurrent access control in methods or class fields.
 *   - Guarantee that functions and _static_ methods are thread safe.
 *
 * As a consequence, it is _not_ thread-safe to access a monitor's member, be it
 * a method or a field, from different threads concurrently.  The easiest way to
 * implement thread-safety when using `libfswatch`, therefore, is segregating
 * access to each monitor instance from a different thread.
 *
 * Similarly, the C API has been designed in order to provide the same
 * guarantees offered by the C++ API:
 *
 *   - Concurrently manipulating different monitoring sessions is thread safe.
 *   - Concurrently manipulating the same monitoring session is _not_ thread
 *     safe.
 *
 * @section cpp11 C++11
 *
 * There is an additional limitation which affects the C library only: the C
 * binding implementation internally uses C++11 classes and keywords to provide
 * the aforementioned guarantees.  If compiler or library support is not found
 * when building `libfswatch` the library will still build, but those guarantees
 * will _not_ be honoured.  A warning such as the following will appear in the
 * output of `configure` to inform the user:
 *
 *     configure: WARNING: libfswatch is not thread-safe because the current
 *     combination of compiler and libraries do not support the thread_local
 *     storage specifier.
 *
 * @section bug-reports Reporting Bugs and Suggestions
 *
 * If you find problems or have suggestions about this program or this manual,
 * please report them as new issues in the official GitHub repository of
 * `fswatch` at https://github.com/emcrisostomo/fswatch.  Please, read the
 * `CONTRIBUTING.md` file for detailed instructions on how to contribute to
 * `fswatch`.
 */
/**
 * @page cpp-api C++ API
 *
 * The C++ API provides users an easy to use, object-oriented interface to a
 * wide range of file monitoring APIs.  This API provides a common facade to a
 * set of heterogeneous APIs that not only greatly simplifies their usage, but
 * provides an indirection layer that makes applications more portable: as far
 * as there is an available monitor in another platform, an existing application
 * will just work.
 *
 * In reality, a monitor may have platform-specific behaviours that should be
 * taken into account when writing portable applications using this library.
 * This differences complicate the task of writing portable applications that
 * are truly independent of the file monitoring API they may be using. However,
 * monitors try to ‘compensate’ for any behavioural difference across
 * implementations.
 *
 * The fsw::monitor class is the basic type of the C++ API: it defines the
 * interface of every monitor and provides common functionality to inheritors of
 * this class, such as:
 *
 *   - Configuration and life cycle (fsw::monitor).
 *   - Event filtering (fsw::monitor).
 *   - Path filtering (fsw::monitor).
 *   - Monitor registration (fsw::monitor_factory).
 *   - Monitor discovery (fsw::monitor_factory).
 *
 * @section Usage
 *
 * The typical usage pattern of this API is similar to the following:
 *
 *   - An instance of a monitor is either created directly or through the
 *     factory (fsw::monitor_factory).
 *   - The monitor is configured (fsw::monitor).
 *   - The monitor is run and change events are waited for
 *     (fsw::monitor::start()).
 *
 * @section cpp-example Example
 *
 *     // Create the default platform monitor
 *     monitor *active_monitor =
 *       monitor_factory::create_monitor(fsw_monitor_type::system_default_monitor_type,
 *                                       paths,
 *                                       process_events);
 *
 *     // Configure the monitor
 *     active_monitor->set_properties(monitor_properties);
 *     active_monitor->set_allow_overflow(allow_overflow);
 *     active_monitor->set_latency(latency);
 *     active_monitor->set_recursive(recursive);
 *     active_monitor->set_directory_only(directory_only);
 *     active_monitor->set_event_type_filters(event_filters);
 *     active_monitor->set_filters(filters);
 *     active_monitor->set_follow_symlinks(follow_symlinks);
 *     active_monitor->set_watch_access(watch_access);
 *
 *     // Start the monitor
 *     active_monitor->start();
 */
/**
 * @page c-api C API
 *
 * The C API, whose main header file is libfswatch.h, is a C-compatible
 * lightweight wrapper around the C++ API that provides an easy to use binding
 * to C clients.  The central type in the C API is the _monitoring session_, an
 * opaque type identified by a handle of type ::FSW_HANDLE that can be
 * manipulated using the C functions of this library.
 *
 * Session-modifying API calls (such as fsw_add_path()) will take effect the
 * next time a monitor is started with fsw_start_monitor().
 *
 * @section cpp-to-c Translating the C++ API to C
 *
 * The conventions used to translate C++ types into C types are simple:
 *
 *   - `std::string` is represented as a `NUL`-terminated `char *`.
 *
 *   - Lists are represented as arrays whose length is specified in a separate
 *     field.
 *
 *   - More complex types are usually translated as a `struct` containing data
 *     fields and a set of functions to operate on it.
 *
 * @section c-thread-safety Thread Safety
 *
 * If the compiler and the C++ library used to build `libfswatch` support the
 * `thread_local` storage specifier then this API is thread safe and a
 * different state is maintained on a per-thread basis.
 *
 * Even when `thread_local` is not available, manipulating _different_
 * monitoring sessions concurrently from different threads is thread safe, since
 * they share no data.
 *
 * @section c-lib-init Library Initialization
 *
 * Before calling any library method, the library must be initialized by calling
 * the fsw_init_library() function:
 *
 *     // Initialize the library
 *     FSW_STATUS ret = fsw_init_library();
 *     if (ret != FSW_OK)
 *     {
 *       exit(1);
 *     }
 *
 * @section c-status-code Status Codes and Errors
 *
 * Most API functions return a status code of type ::FSW_STATUS, defined in the
 * error.h header.  A successful API call returns ::FSW_OK and the last error
 * can be obtained calling the fsw_last_error() function.
 *
 * @section c-example Example
 *
 * This is a basic example of how a monitor session can be constructed and run
 * using the C API.  To be valid, a session needs at least the following
 * information:
 *
 *   - A path to watch.
 *   - A _callback_ to process the events sent by the monitor.
 *
 * The next code fragment shows how to create and start a basic monitoring
 * session (error checking code was omitted):
 *
 *     // Initialize the library
 *     fsw_init_library();
 *
 *     // Use the default monitor.
 *     const FSW_HANDLE handle = fsw_init_session();
 *     fsw_add_path(handle, "my/path");
 *     fsw_set_callback(handle, my_callback);
 *     fsw_start_monitor(handle);
 */
/**
 * @page history History
 *
 * @section v1011 10:1:1
 *
 *   - Migrate usages of POSIX regular expressions (<regex.h>) to the C++11
 *    regex library (<regex>).
 *   - Wrong error message is printed when inotify event queue overflows.
 *
 * @section v1001 10:0:1
 *
 *   - Fix C99 compatibility in cevent.h by not implying enum.
 *   - Free session memory.
 *   - Fix segmentation fault when starting monitor.
 *   - Add fsw_is_running() function to the C API to check that a monitor is
 *     running.
 *   - Fix stop sequence in fsw::fsevents_monitor::run() and in
 *     fsw::fsevents_monitor::on_stop().
 *
 * @section v900 9:0:0
 *
 *   - Add fsw::monitor_filter::read_from_file() to load filters from a file.
 *   - Add fsw_stop_monitor() function to stop a running monitor.
 *   - Change FSW_HANDLE type.
 *
 * @section v600 8:0:2
 *
 *   - Add a mutex to protect the fsw::monitor::notify_events() method.
 *   - Substitute C++ header names with C names in C headers.
 *
 * @section v600 7:0:1
 *
 *   - fsw::monitor::~monitor(): update to invoke fsw::monitor::stop().
 *   - Close resources in monitor::on_stop() instead of doing it in destructors.
 *   - Add inactivity callback.
 *
 * @section v600 6:0:0
 *
 *   - fsw::monitor::stop(): added.
 *   - fsw::monitor::monitor(): update to move paths instead of copying them.
 *   - fsw::monitor_factory::exists_type(const std::string&): added.
 *   - fsw::monitor_factory::exists_type(const fsw_monitor_type&): added.
 *   - fsw::fsevents_monitor::set_numeric_event(): removed.
 *   - fsw::string_utils::string_from_format: added.
 *   - fsw::string_utils::vstring_from_format: added.
 *
 * @section v502 5:0:2
 *
 *   - A monitor based on the Solaris/Illumos File Events Notification API has
 *     been added.
 *
 *   - The possibility of watching for directories only during a recursive scan.
 *     This feature helps reducing the number of open file descriptors if a
 *     generic change event for a directory is acceptable instead of events on
 *     directory children.
 *
 *   - fsw::fen_monitor: added to provide a monitor based on the Solaris/Illumos
 *     File Events Notification API.
 *
 *   - fsw::monitor::set_directory_only(): added to set a flag to only watch
 *     directories during a recursive scan.
 *
 *   - fsw_set_directory_only(): added to set a flag to only watch directories
 *     during a recursive scan.
 *
 *   - fsw_logf_perror(): added to log a `printf()`-style message using
 *     `perror()`.
 *
 * @section v401 4:0:1
 *
 *   - fsw::windows_monitor: a monitor for Microsoft Windows was added.
 *
 *   - A logging function has been added to log verbose messages.
 *
 *   - A family of functions and macros have been added to log diagnostic
 *     messages:
 *
 *     - fsw_flog()
 *     - fsw_logf()
 *     - fsw_flogf()
 *     - fsw_log_perror()
 *     - ::FSW_LOG
 *     - ::FSW_ELOG
 *     - ::FSW_LOGF
 *     - ::FSW_ELOGF
 *     - ::FSW_FLOGF
 *
 * @section v300 3:0:0
 *
 *   - Added ability to filter events _by type_:
 *
 *     - fsw::monitor::add_event_type_filter()
 *     - fsw::monitor::set_event_type_filters()
 *
 *   - fsw::monitor::notify_events(): added to centralize event filtering and
 *     dispatching into the monitor base class.
 *
 *   - Added ability to get event types by name and stringify them:
 *
 *     - fsw::event::get_event_flag_by_name()
 *     - fsw::event::get_event_flag_name()
 *     - fsw_get_event_flag_by_name()
 *     - fsw_get_event_flag_name()
 *
 *   - fsw_event_type_filter: added to represent an event type filter.
 *
 *   - ::FSW_ERR_UNKNOWN_VALUE: added error code.
 *
 *   - fsw_add_event_type_filter(): added to add an event type filter.
 */
/**
 * @page path-filtering Path Filtering
 *
 * A _path filter_ (fsw::monitor_filter) can be used to filter event paths.  A
 * filter type (::fsw_filter_type) determines whether the filter regular
 * expression is used to include and exclude paths from the list of the events
 * processed by the library.  `libfswatch` processes filters this way:
 *
 *   - If a path matches an including filter, the path is _accepted_ no matter
 *     any other filter.
 *
 *   - If a path matches an excluding filter, the path is _rejected_.
 *
 *   - If a path matches no  lters, the path is _accepted_.
 *
 * Said another way:
 *
 *   - All paths are accepted _by default_, unless an exclusion filter says
 *     otherwise.
 *
 *   - Inclusion filters may override any other exclusion filter.
 *
 *   - The order in the filter definition has no effect.
 */

#ifdef HAVE_CONFIG_H
#  include "libfswatch_config.h"
#endif

#include "gettext_defs.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
#include <map>
#include "libfswatch.h"
#include "../c++/libfswatch_map.hpp"
#include "../c++/filter.hpp"
#include "../c++/monitor.hpp"
#include "../c++/monitor_factory.hpp"
#include "../c++/libfswatch_exception.hpp"

using namespace std;
using namespace fsw;

typedef struct FSW_SESSION
{
  vector<string> paths;
  fsw_monitor_type type;
  fsw::monitor *monitor;
  FSW_CEVENT_CALLBACK callback;
  double latency;
  bool allow_overflow;
  bool recursive;
  bool directory_only;
  bool follow_symlinks;
  vector<monitor_filter> filters;
  vector<fsw_event_type_filter> event_type_filters;
  map<string, string> properties;
  void *data;
} FSW_SESSION;

static bool fsw_libfswatch_verbose = false;
static FSW_THREAD_LOCAL FSW_STATUS last_error;

// Forward declarations.
static FSW_EVENT_CALLBACK libfsw_cpp_callback_proxy;
static FSW_SESSION *get_session(const FSW_HANDLE handle);
static int create_monitor(FSW_HANDLE handle, const fsw_monitor_type type);
static FSW_STATUS fsw_set_last_error(const int error);

/*
 * Library initialization routine.  Currently, libfswatch only initializes
 * gettext.
 */
FSW_STATUS fsw_init_library()
{
  // Trigger gettext operations
#ifdef ENABLE_NLS
  bindtextdomain(PACKAGE, LOCALEDIR);
#endif

  return FSW_OK;
}

typedef struct fsw_callback_context
{
  FSW_HANDLE handle;
  FSW_CEVENT_CALLBACK callback;
  void *data;
} fsw_callback_context;

void libfsw_cpp_callback_proxy(const std::vector<event>& events,
                               void *context_ptr)
{
  // TODO: A C friendly error handler should be notified instead of throwing an exception.
  if (!context_ptr)
    throw int(FSW_ERR_MISSING_CONTEXT);

  const fsw_callback_context *context = static_cast<fsw_callback_context *> (context_ptr);

  fsw_cevent *const cevents = static_cast<fsw_cevent *> (malloc(
    sizeof(fsw_cevent) * events.size()));

  if (cevents == nullptr)
    throw int(FSW_ERR_MEMORY);

  for (unsigned int i = 0; i < events.size(); ++i)
  {
    fsw_cevent *cevt = &cevents[i];
    const event& evt = events[i];

    // Copy event into C event wrapper.
    const string path = evt.get_path();

    // Copy std::string into char * buffer and null-terminate it.
    cevt->path = static_cast<char *> (malloc(
      sizeof(char *) * (path.length() + 1)));
    if (!cevt->path) throw int(FSW_ERR_MEMORY);

    strncpy(cevt->path, path.c_str(), path.length());
    cevt->path[path.length()] = '\0';
    cevt->evt_time = evt.get_time();

    const vector<fsw_event_flag> flags = evt.get_flags();
    cevt->flags_num = flags.size();

    if (!cevt->flags_num) cevt->flags = nullptr;
    else
    {
      cevt->flags =
        static_cast<fsw_event_flag *> (
          malloc(sizeof(fsw_event_flag) * cevt->flags_num));
      if (!cevt->flags) throw int(FSW_ERR_MEMORY);
    }

    for (unsigned int e = 0; e < cevt->flags_num; ++e)
    {
      cevt->flags[e] = flags[e];
    }
  }

  // TODO manage C++ exceptions from C code
  (*(context->callback))(cevents, events.size(), context->data);

  // Deallocate memory allocated by events.
  for (unsigned int i = 0; i < events.size(); ++i)
  {
    fsw_cevent *cevt = &cevents[i];

    if (cevt->flags) free(static_cast<void *> (cevt->flags));
    free(static_cast<void *> (cevt->path));
  }

  free(static_cast<void *> (cevents));
}

FSW_HANDLE fsw_init_session(const fsw_monitor_type type)
{
  FSW_SESSION *session = new FSW_SESSION{};
  session->type = type;

  return session;
}

int create_monitor(const FSW_HANDLE handle, const fsw_monitor_type type)
{
  try
  {
    FSW_SESSION *session = get_session(handle);

    // Check sufficient data is present to build a monitor.
    if (!session->callback)
      return fsw_set_last_error(int(FSW_ERR_CALLBACK_NOT_SET));

    if (session->monitor)
      return fsw_set_last_error(int(FSW_ERR_MONITOR_ALREADY_EXISTS));

    if (!session->paths.size())
      return fsw_set_last_error(int(FSW_ERR_PATHS_NOT_SET));

    fsw_callback_context *context_ptr = new fsw_callback_context;
    context_ptr->handle = session;
    context_ptr->callback = session->callback;
    context_ptr->data = session->data;

    monitor *current_monitor = monitor_factory::create_monitor(type,
                                                               session->paths,
                                                               libfsw_cpp_callback_proxy,
                                                               context_ptr);
    session->monitor = current_monitor;
  }
  catch (libfsw_exception& ex)
  {
    return fsw_set_last_error(int(ex));
  }
  catch (int error)
  {
    return fsw_set_last_error(error);
  }

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_add_path(const FSW_HANDLE handle, const char *path)
{
  if (!path)
    return fsw_set_last_error(int(FSW_ERR_INVALID_PATH));

  FSW_SESSION *session = get_session(handle);
  session->paths.push_back(path);

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_add_property(const FSW_HANDLE handle,
                            const char *name,
                            const char *value)
{
  if (!name || !value)
    return fsw_set_last_error(FSW_ERR_INVALID_PROPERTY);

  FSW_SESSION *session = get_session(handle);
  session->properties[name] = value;

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_set_callback(const FSW_HANDLE handle,
                            const FSW_CEVENT_CALLBACK callback,
                            void *data)
{
  if (!callback)
    return fsw_set_last_error(int(FSW_ERR_INVALID_CALLBACK));

  FSW_SESSION *session = get_session(handle);
  session->callback = callback;
  session->data = data;

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_set_allow_overflow(const FSW_HANDLE handle,
                                  const bool allow_overflow)
{
  FSW_SESSION *session = get_session(handle);
  session->allow_overflow = allow_overflow;

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_set_latency(const FSW_HANDLE handle, const double latency)
{
  if (latency < 0)
    return fsw_set_last_error(int(FSW_ERR_INVALID_LATENCY));

  FSW_SESSION *session = get_session(handle);
  session->latency = latency;

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_set_recursive(const FSW_HANDLE handle, const bool recursive)
{
  FSW_SESSION *session = get_session(handle);
  session->recursive = recursive;

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_set_directory_only(const FSW_HANDLE handle,
                                  const bool directory_only)
{
  FSW_SESSION *session = get_session(handle);
  session->directory_only = directory_only;

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_set_follow_symlinks(const FSW_HANDLE handle,
                                   const bool follow_symlinks)
{
  FSW_SESSION *session = get_session(handle);
  session->follow_symlinks = follow_symlinks;

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_add_event_type_filter(const FSW_HANDLE handle,
                                     const fsw_event_type_filter event_type)
{
  FSW_SESSION *session = get_session(handle);
  session->event_type_filters.push_back(event_type);

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_add_filter(const FSW_HANDLE handle,
                          const fsw_cmonitor_filter filter)
{
  FSW_SESSION *session = get_session(handle);
  session->filters.push_back(
    {filter.text, filter.type, filter.case_sensitive, filter.extended});

  return fsw_set_last_error(FSW_OK);
}

bool fsw_is_running(const FSW_HANDLE handle)
{
  FSW_SESSION *session = get_session(handle);

  if (!session->monitor)
    return false;

  return session->monitor->is_running();
}

FSW_STATUS fsw_start_monitor(const FSW_HANDLE handle)
{
  try
  {
    FSW_SESSION *session = get_session(handle);

    if (!session->monitor)
    {
      FSW_STATUS ret = create_monitor(handle, session->type);

      if (ret != FSW_OK)
        return fsw_set_last_error(ret);
    }

    if (session->monitor == nullptr) // create_monitor returned OK, but monitor were not created
      return fsw_set_last_error(FSW_ERR_UNKNOWN_MONITOR_TYPE);

    if (session->monitor->is_running())
      return fsw_set_last_error(int(FSW_ERR_MONITOR_ALREADY_RUNNING));

    session->monitor->set_allow_overflow(session->allow_overflow);
    session->monitor->set_filters(session->filters);
    session->monitor->set_event_type_filters(session->event_type_filters);
    session->monitor->set_follow_symlinks(session->follow_symlinks);
    if (session->latency) session->monitor->set_latency(session->latency);
    session->monitor->set_recursive(session->recursive);
    session->monitor->set_directory_only(session->directory_only);

    session->monitor->start();
  }
  catch (libfsw_exception& ex)
  {
    return fsw_set_last_error(int(ex));
  }
  catch (int error)
  {
    return fsw_set_last_error(error);
  }

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_stop_monitor(const FSW_HANDLE handle)
{
  try
  {
    FSW_SESSION *session = get_session(handle);

    if (session->monitor == nullptr)
      return fsw_set_last_error(int(FSW_ERR_UNKNOWN_MONITOR_TYPE));

    if (!session->monitor->is_running())
      return fsw_set_last_error(int(FSW_OK));

    session->monitor->stop();
  }
  catch (libfsw_exception& ex)
  {
    return fsw_set_last_error(int(ex));
  }
  catch (int error)
  {
    return fsw_set_last_error(error);
  }

  return fsw_set_last_error(FSW_OK);
}

FSW_STATUS fsw_destroy_session(const FSW_HANDLE handle)
{
  int ret = FSW_OK;

  try
  {
    FSW_SESSION *session = get_session(handle);

    if (session->monitor)
    {
      if (session->monitor->is_running())
      {
        return fsw_set_last_error(FSW_ERR_MONITOR_ALREADY_RUNNING);
      }

      void *context = session->monitor->get_context();

      if (!context)
      {
        session->monitor->set_context(nullptr);
        delete static_cast<fsw_callback_context *> (context);
      }
      delete session->monitor;
    }

    delete session;
  }
  catch (int error)
  {
    ret = error;
  }

  return fsw_set_last_error(ret);
}

FSW_SESSION *get_session(const FSW_HANDLE handle)
{
  return handle;
}

FSW_STATUS fsw_set_last_error(const FSW_STATUS error)
{
  last_error = error;

  return last_error;
}

FSW_STATUS fsw_last_error()
{
  return last_error;
}

bool fsw_is_verbose()
{
  return fsw_libfswatch_verbose;
}

void fsw_set_verbose(bool verbose)
{
  fsw_libfswatch_verbose = verbose;
}
