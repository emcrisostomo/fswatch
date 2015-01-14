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
#  include "libfswatch_config.h"
#endif

#include "gettext_defs.h"
#include <iostream>
#ifdef HAVE_CXX_MUTEX
#  include <mutex>
#  include <atomic>
#endif
#include <ctime>
#include <stdlib.h>
#include <cstring>
#include <memory>
#include "libfswatch.h"
#include "../c++/libfswatch_map.h"
#include "../c++/filter.h"
#include "../c++/monitor.h"
#include "../c++/libfswatch_exception.h"

using namespace std;
using namespace fsw;

typedef struct FSW_SESSION
{
  FSW_HANDLE handle;
  vector<string> paths;
  fsw_monitor_type type;
  fsw::monitor * monitor;
  FSW_CEVENT_CALLBACK callback;
  double latency;
  bool recursive;
  bool follow_symlinks;
  vector<monitor_filter> filters;
#ifdef HAVE_CXX_MUTEX
  atomic<bool> running;
#endif
} FSW_SESSION;

static bool srand_initialized = false;

#ifdef HAVE_CXX_UNIQUE_PTR
static fsw_hash_map<FSW_HANDLE, unique_ptr<FSW_SESSION>> sessions;
#else
static fsw_hash_map<FSW_HANDLE, FSW_SESSION *> sessions;
#endif

#ifdef HAVE_CXX_MUTEX
#  ifdef HAVE_CXX_UNIQUE_PTR
static fsw_hash_map<FSW_HANDLE, unique_ptr<mutex>> session_mutexes;
#  else
static fsw_hash_map<FSW_HANDLE, mutex *> session_mutexes;
#  endif
static std::mutex session_mutex;
#endif

#if defined(HAVE_CXX_THREAD_LOCAL)
static FSW_THREAD_LOCAL unsigned int last_error;
#endif

// Default library callback.
static FSW_EVENT_CALLBACK libfsw_cpp_callback_proxy;
static FSW_SESSION * get_session(const FSW_HANDLE handle);

static int create_monitor(FSW_HANDLE handle, const fsw_monitor_type type);
static FSW_STATUS fsw_set_last_error(const int error);

/*
 * Library initialization routine.  Currently, libfswatch only initializes
 * gettext.
 */
int fsw_init_library()
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
} fsw_callback_context;

void libfsw_cpp_callback_proxy(const std::vector<event> & events,
                               void * context_ptr)
{
  // TODO: A C friendly error handler should be notified instead of throwing an exception.
  if (!context_ptr)
    throw int(FSW_ERR_MISSING_CONTEXT);

  const fsw_callback_context * context = static_cast<fsw_callback_context *> (context_ptr);

  fsw_cevent * const cevents = static_cast<fsw_cevent *> (::malloc(sizeof (fsw_cevent) * events.size()));

  if (cevents == nullptr)
    throw int(FSW_ERR_MEMORY);

  for (unsigned int i = 0; i < events.size(); ++i)
  {
    fsw_cevent * cevt = &cevents[i];
    const event & evt = events[i];

    // Copy event into C event wrapper.
    const string path = evt.get_path();

    // Copy std::string into char * buffer and null-terminate it.
    cevt->path = static_cast<char *> (::malloc(sizeof (char *) * (path.length() + 1)));
    if (!cevt->path) throw int(FSW_ERR_MEMORY);

    ::strncpy(cevt->path, path.c_str(), path.length());
    cevt->path[path.length()] = '\0';

    cevt->evt_time = evt.get_time();

    const vector<fsw_event_flag> flags = evt.get_flags();
    cevt->flags_num = flags.size();

    if (!cevt->flags_num) cevt->flags = nullptr;
    else
    {
      cevt->flags = static_cast<fsw_event_flag *> (::malloc(sizeof (fsw_event_flag) * cevt->flags_num));
      if (!cevt->flags) throw int(FSW_ERR_MEMORY);
    }

    for (unsigned int e = 0; e < cevt->flags_num; ++e)
    {
      cevt->flags[e] = flags[e];
    }
  }

  // TODO manage C++ exceptions from C code
  (*(context->callback))(cevents, events.size());

  // Deallocate memory allocated by events.
  for (unsigned int i = 0; i < events.size(); ++i)
  {
    fsw_cevent * cevt = &cevents[i];

    if (cevt->flags) ::free(static_cast<void *> (cevt->flags));
    ::free(static_cast<void *> (cevt->path));
  }

  ::free(static_cast<void *> (cevents));
}

FSW_HANDLE fsw_init_session(const fsw_monitor_type type)
{
#ifdef HAVE_CXX_MUTEX
  std::lock_guard<std::mutex> session_lock(session_mutex);
#endif

  if (!srand_initialized)
  {
    srand(time(nullptr));
    srand_initialized = true;
  }

  int handle;

  do
  {
    handle = rand();
  }
  while (sessions.find(handle) != sessions.end());

  FSW_SESSION *session = new FSW_SESSION{};

  session->handle = handle;
  session->type = type;

  // Store the handle and a mutex to guard access to session instances.
#ifdef HAVE_CXX_UNIQUE_PTR
  sessions[handle] = unique_ptr<FSW_SESSION>(session);
#else
  sessions[handle] = session;
#endif

#ifdef HAVE_CXX_MUTEX
#  ifdef HAVE_CXX_UNIQUE_PTR
  session_mutexes[handle] = unique_ptr<mutex>(new mutex);
#  else
  session_mutexes[handle] = new mutex;
#  endif
#endif

  return handle;
}

int create_monitor(const FSW_HANDLE handle, const fsw_monitor_type type)
{
  try
  {
    FSW_SESSION * session = get_session(handle);

    // Check sufficient data is present to build a monitor.
    if (!session->callback)
      return fsw_set_last_error(int(FSW_ERR_CALLBACK_NOT_SET));

    if (session->monitor)
      return fsw_set_last_error(int(FSW_ERR_MONITOR_ALREADY_EXISTS));

    if (!session->paths.size())
      return fsw_set_last_error(int(FSW_ERR_PATHS_NOT_SET));

    fsw_callback_context * context_ptr = new fsw_callback_context;
    context_ptr->callback = session->callback;
    context_ptr->handle = session->handle;

    monitor * current_monitor = monitor::create_monitor(type,
                                                        session->paths,
                                                        libfsw_cpp_callback_proxy,
                                                        context_ptr);
    session->monitor = current_monitor;
  }
  catch (libfsw_exception ex)
  {
    return fsw_set_last_error(int(ex));
  }
  catch (int error)
  {
    return fsw_set_last_error(error);
  }

  return fsw_set_last_error(FSW_OK);
}

int fsw_add_path(const FSW_HANDLE handle, const char * path)
{
  if (!path)
    return fsw_set_last_error(int(FSW_ERR_INVALID_PATH));

  try
  {
#ifdef HAVE_CXX_MUTEX
    std::lock_guard<std::mutex> session_lock(session_mutex);
#endif
    FSW_SESSION * session = get_session(handle);

    session->paths.push_back(path);
  }
  catch (int error)
  {
    return fsw_set_last_error(error);
  }

  return fsw_set_last_error(FSW_OK);
}

int fsw_set_callback(const FSW_HANDLE handle, const FSW_CEVENT_CALLBACK callback)
{
  if (!callback)
    return fsw_set_last_error(int(FSW_ERR_INVALID_CALLBACK));

  try
  {
#ifdef HAVE_CXX_MUTEX
    std::lock_guard<std::mutex> session_lock(session_mutex);
#endif
    FSW_SESSION * session = get_session(handle);

    session->callback = callback;
  }
  catch (int error)
  {
    return fsw_set_last_error(error);
  }

  return fsw_set_last_error(FSW_OK);
}

int fsw_set_latency(const FSW_HANDLE handle, const double latency)
{
  if (latency < 0)
    return fsw_set_last_error(int(FSW_ERR_INVALID_LATENCY));

  try
  {
#ifdef HAVE_CXX_MUTEX
    std::lock_guard<std::mutex> session_lock(session_mutex);
#endif
    FSW_SESSION * session = get_session(handle);

    session->latency = latency;
  }
  catch (int error)
  {
    return fsw_set_last_error(error);
  }

  return fsw_set_last_error(FSW_OK);
}

int fsw_set_recursive(const FSW_HANDLE handle, const bool recursive)
{
  try
  {
#ifdef HAVE_CXX_MUTEX
    std::lock_guard<std::mutex> session_lock(session_mutex);
#endif
    FSW_SESSION * session = get_session(handle);

    session->recursive = recursive;
  }
  catch (int error)
  {
    return fsw_set_last_error(error);
  }

  return fsw_set_last_error(FSW_OK);
}

int fsw_set_follow_symlinks(const FSW_HANDLE handle,
                            const bool follow_symlinks)
{
  try
  {
#ifdef HAVE_CXX_MUTEX
    std::lock_guard<std::mutex> session_lock(session_mutex);
#endif
    FSW_SESSION * session = get_session(handle);

    session->follow_symlinks = follow_symlinks;
  }
  catch (int error)
  {
    return fsw_set_last_error(error);
  }

  return fsw_set_last_error(FSW_OK);
}

int fsw_add_filter(const FSW_HANDLE handle,
                   const fsw_cmonitor_filter filter)
{
  try
  {
#ifdef HAVE_CXX_MUTEX
    std::lock_guard<std::mutex> session_lock(session_mutex);
#endif
    FSW_SESSION * session = get_session(handle);

    session->filters.push_back({filter.text, filter.type, filter.case_sensitive, filter.extended});
  }
  catch (int error)
  {
    return fsw_set_last_error(error);
  }

  return fsw_set_last_error(FSW_OK);
}

#ifdef HAVE_CXX_MUTEX

template <typename T>
class monitor_start_guard
{
  atomic<T> & a;
  T val;

public:

  monitor_start_guard(atomic<T> & a,
                      T val,
                      memory_order sync = memory_order_seq_cst)
    : a(a), val(val)
  {
  }

  ~monitor_start_guard()
  {
    a.store(val, memory_order_release);
  }
};
#endif

int fsw_start_monitor(const FSW_HANDLE handle)
{
  try
  {
#ifdef HAVE_CXX_MUTEX
    unique_lock<mutex> session_lock(session_mutex, defer_lock);
    session_lock.lock();
#endif

    FSW_SESSION * session = get_session(handle);

#ifdef HAVE_CXX_MUTEX
    if (session->running.load(memory_order_acquire))
      return fsw_set_last_error(int(FSW_ERR_MONITOR_ALREADY_RUNNING));

#  ifdef HAVE_CXX_UNIQUE_PTR
    unique_ptr<mutex> & sm = session_mutexes.at(handle);
    lock_guard<mutex> lock_sm(*sm.get());
#  else
    mutex * sm = session_mutexes.at(handle);
    lock_guard<mutex> lock_sm(*sm);
#  endif

    session_lock.unlock();
#endif

    if (!session->monitor)
      create_monitor(handle, session->type);

    session->monitor->set_filters(session->filters);
    session->monitor->set_follow_symlinks(session->follow_symlinks);
    if (session->latency) session->monitor->set_latency(session->latency);
    session->monitor->set_recursive(session->recursive);

#ifdef HAVE_CXX_MUTEX
    session->running.store(true, memory_order_release);
    monitor_start_guard<bool> guard(session->running, false);
#endif

    session->monitor->start();
  }
  catch (int error)
  {
    return fsw_set_last_error(error);
  }

  return fsw_set_last_error(FSW_OK);
}

int fsw_destroy_session(const FSW_HANDLE handle)
{
  int ret = FSW_OK;

  try
  {
#ifdef HAVE_CXX_MUTEX
    std::lock_guard<std::mutex> session_lock(session_mutex);
#endif
    FSW_SESSION * session = get_session(handle);

#ifdef HAVE_CXX_MUTEX
#  ifdef HAVE_CXX_UNIQUE_PTR
    const unique_ptr<mutex> & sm = session_mutexes[handle];
    lock_guard<mutex> sm_lock(*sm.get());
#  else
    mutex * sm = session_mutexes[handle];
    lock_guard<mutex> sm_lock(*sm);
#  endif
#endif

    if (session->monitor)
    {
      void * context = session->monitor->get_context();

      if (!context)
      {
        session->monitor->set_context(nullptr);
        delete static_cast<fsw_callback_context *> (context);
      }
      delete session->monitor;
    }

    sessions.erase(handle);
  }
  catch (int error)
  {
    ret = error;
  }

  #ifdef HAVE_CXX_MUTEX
    session_mutexes.erase(handle);
  #endif

  return fsw_set_last_error(FSW_OK);
}

FSW_SESSION * get_session(const FSW_HANDLE handle)
{
  if (sessions.find(handle) == sessions.end())
    throw int(FSW_ERR_SESSION_UNKNOWN);

#ifdef HAVE_CXX_UNIQUE_PTR
  return sessions[handle].get();
#else
  return sessions[handle];
#endif
}

int fsw_set_last_error(const int error)
{
#if defined(HAVE_CXX_THREAD_LOCAL)
  last_error = error;
#endif

  return error;
}

int fsw_last_error()
{
#if defined(HAVE_CXX_THREAD_LOCAL)
  return last_error;
#else
  return fsw_set_last_error(FSW_ERR_UNSUPPORTED_OPERATION);
#endif
}

bool fsw_is_verbose()
{
  return false;
}
