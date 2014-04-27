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
#include "kqueue_monitor.h"

#ifdef HAVE_SYS_EVENT_H

#include "fswatch_exception.h"
#include "fswatch_log.h"
#include "path_utils.h"
#include <iostream>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

typedef struct KqueueFlagType
{
  uint32_t flag;
  event_flag type;
} KqueueFlagType;

static vector<KqueueFlagType> create_flag_type_vector();
static const vector<KqueueFlagType> event_flag_type = create_flag_type_vector();

vector<KqueueFlagType> create_flag_type_vector()
{
  vector<KqueueFlagType> flags;
  flags.push_back({NOTE_DELETE, event_flag::Removed});
  flags.push_back({NOTE_WRITE, event_flag::Updated});
  flags.push_back({NOTE_EXTEND, event_flag::PlatformSpecific});
  flags.push_back({NOTE_ATTRIB, event_flag::AttributeModified});
  flags.push_back({NOTE_LINK, event_flag::Link});
  flags.push_back({NOTE_RENAME, event_flag::Renamed});
  flags.push_back({NOTE_REVOKE, event_flag::PlatformSpecific});

  return flags;
}

kqueue_monitor::kqueue_monitor(vector<string> paths_to_monitor,
                               EVENT_CALLBACK callback) :
  monitor(paths_to_monitor, callback)
{
}

kqueue_monitor::~kqueue_monitor()
{
  if (kq != -1) ::close(kq);
}

static vector<event_flag> decode_flags(uint32_t flag)
{
  vector<event_flag> evt_flags;

  for (const KqueueFlagType &type : event_flag_type)
  {
    if (flag & type.flag)
    {
      evt_flags.push_back(type.type);
    }
  }

  return evt_flags;
}

static struct timespec create_timespec_from_latency(double latency)
{
  double seconds;
  double nanoseconds = modf(latency, &seconds);
  nanoseconds *= 1000000000;

  struct timespec ts;
  ts.tv_sec = seconds;
  ts.tv_nsec = nanoseconds;

  return ts;
}

bool kqueue_monitor::is_path_watched(const string & path)
{
  return descriptors_by_file_name.find(path) != descriptors_by_file_name.end();
}

bool kqueue_monitor::add_watch(const string & path, const struct stat &fd_stat)
{
  // check if the path is already watched and if it is,
  // skip it and return false.
  if (is_path_watched(path))
  {
    return false;
  }

  int o_flags = 0;
#ifdef O_SYMLINK
  o_flags |= O_SYMLINK;
#elif defined(O_NOFOLLOW)
  o_flags |= O_NOFOLLOW;
#endif
#ifdef O_EVTONLY
  o_flags |= O_EVTONLY;
#else
  o_flags |= O_RDONLY;
#endif

  int fd = ::open(path.c_str(), o_flags);

  if (fd == -1)
  {
    string err = string("Cannot open ") + path;
    fsw_perror(err.c_str());

    return false;
  }

  // if the descriptor could be opened, track it
  descriptors_by_file_name[path] = fd;
  file_names_by_descriptor[fd] = path;
  file_modes[fd] = fd_stat.st_mode;

  return true;
}

bool kqueue_monitor::scan(const string &path)
{
  if (!accept_path(path))
    return true;

  struct stat fd_stat;
  if (!stat_path(path, fd_stat)) return false;

  if (follow_symlinks && S_ISLNK(fd_stat.st_mode))
  {
    string link_path;
    if (read_link_path(path, link_path))
      return scan(link_path);

    return false;
  }
  else if (!add_watch(path, fd_stat))
  {
    return false;
  }

  if (!recursive) return true;
  if (!S_ISDIR(fd_stat.st_mode)) return true;

  vector<string> dirs_to_process;
  dirs_to_process.push_back(path);

  while (dirs_to_process.size() > 0)
  {
    const string current_dir = dirs_to_process.back();
    dirs_to_process.pop_back();

    vector<string> children;
    get_directory_children(current_dir, children);

    for (string &child : children)
    {
      if (child.compare(".") == 0 || child.compare("..") == 0) continue;

      const string fqpath = current_dir + "/" + child;

      if (!accept_path(fqpath)) continue;
      if (!stat_path(fqpath, fd_stat)) continue;

      if (follow_symlinks && S_ISLNK(fd_stat.st_mode))
      {
        string link_path;
        if (read_link_path(fqpath, link_path))
        {
          scan(link_path);
        }
        continue;
      }
      else if (!add_watch(fqpath, fd_stat)) continue;

      if (S_ISDIR(fd_stat.st_mode)) dirs_to_process.push_back(fqpath);
    }
  }

  return true;
}

void kqueue_monitor::remove_watch(int fd)
{
  string name = file_names_by_descriptor[fd];
  file_names_by_descriptor.erase(fd);
  descriptors_by_file_name.erase(name);
  file_modes.erase(fd);
  ::close(fd);
}

void kqueue_monitor::remove_watch(const string &path)
{
  int fd = descriptors_by_file_name[path];
  descriptors_by_file_name.erase(path);
  file_names_by_descriptor.erase(fd);
  file_modes.erase(fd);
  ::close(fd);
}

void kqueue_monitor::remove_deleted()
{
  auto fd = descriptors_to_remove.begin();

  while (fd != descriptors_to_remove.end())
  {
    remove_watch(*fd);

    descriptors_to_remove.erase(fd++);
  }
}

void kqueue_monitor::rescan_pending()
{
  auto fd = descriptors_to_rescan.begin();

  while (fd != descriptors_to_rescan.end())
  {
    string fd_path = file_names_by_descriptor[*fd];

    // Rescan the hierarchy rooted at fd_path.
    // If the path does not exist any longer, nothing needs to be done since
    // kqueue(2) says:
    //
    // EV_DELETE  Events which are attached to file descriptors are
    //            automatically deleted on the last close of the descriptor.
    //
    // If the descriptor which has vanished is a directory, we don't bother
    // EV_DELETEing all its children the event from kqueue for the same
    // reason.
    remove_watch(fd_path);
    scan(fd_path);

    descriptors_to_rescan.erase(fd++);
  }
}

void kqueue_monitor::scan_root_paths()
{
  for (string &path : paths)
  {
    if (is_path_watched(path)) continue;

    if (!scan(path))
    {
      string err = "Notice: " + path + " cannot be found. Will retry later.\n";
      fsw_log(err.c_str());
    }
  }
}

void kqueue_monitor::initialize_kqueue()
{
  if (kq != -1) throw fsw_exception("kqueue already running.");

  kq = ::kqueue();

  if (kq == -1)
  {
    perror("::kqueue()");
    throw fsw_exception("kqueue failed.");
  }
}

int kqueue_monitor::wait_for_events(const vector<struct kevent> &changes,
                                    vector<struct kevent> &event_list)
{
  struct timespec ts = create_timespec_from_latency(latency);

  int event_num = ::kevent(kq,
                           &changes[0],
                           changes.size(),
                           &event_list[0],
                           event_list.size(),
                           &ts);

  if (event_num == -1)
  {
    perror("::kevent returned -1");
    throw fsw_exception("Invalid event number.");
  }

  return event_num;
}

void kqueue_monitor::process_events(
                                    const vector<struct kevent> &changes,
                                    const vector<struct kevent> &event_list,
                                    int event_num)
{
  time_t curr_time;
  time(&curr_time);
  vector<event> events;

  for (auto i = 0; i < event_num; ++i)
  {
    struct kevent e = event_list[i];

    if (e.flags & EV_ERROR)
    {
      perror("Event with EV_ERROR");
      continue;
    }

    // If a NOTE_DELETE is found or a NOTE_LINK is found on a directory, then
    // the descriptor should be closed and the node rescanned: removing a
    // subtree in *BSD usually result in NOTE_REMOVED | NOTE_LINK being logged
    // for each subdirectory, but sometimes NOTE_WRITE | NOTE_LINK is only
    // observed.  For this reason we mark those descriptors as to be deleted
    // anyway.
    //
    // If a NOTE_RENAME or NOTE_REVOKE flag is found, the file
    // descriptor should probably be closed and the file should be rescanned.
    // If a NOTE_WRITE flag is found and the descriptor is a directory, then
    // the directory needs to be rescanned because at least one file has
    // either been created or deleted.
    if ((e.fflags & NOTE_DELETE))
    {
      descriptors_to_remove.insert(e.ident);
    }
    else if ((e.fflags & NOTE_RENAME) || (e.fflags & NOTE_REVOKE)
             || ((e.fflags & NOTE_WRITE) && S_ISDIR(file_modes[e.ident])))
    {
      descriptors_to_rescan.insert(e.ident);
    }

    // invoke the callback passing every path for which an event has been
    // received with a non empty filter flag.
    if (e.fflags)
    {
      events.push_back({file_names_by_descriptor[e.ident],
                       curr_time,
                       decode_flags(e.fflags)});
    }
  }

  if (events.size())
  {
    callback(events);
  }
}

void kqueue_monitor::run()
{
  initialize_kqueue();

  while (true)
  {
    // remove the deleted descriptors
    remove_deleted();

    // rescan the pending descriptors
    rescan_pending();

    // scan the root paths to check whether someone is missing
    scan_root_paths();

    vector<struct kevent> changes;
    vector<struct kevent> event_list;

    for (const pair<int, string> &fd_path : file_names_by_descriptor)
    {
      struct kevent change;

      EV_SET(&change,
             fd_path.first,
             EVFILT_VNODE,
             EV_ADD | EV_ENABLE | EV_CLEAR,
             NOTE_DELETE | NOTE_EXTEND | NOTE_RENAME | NOTE_WRITE | NOTE_ATTRIB | NOTE_LINK | NOTE_REVOKE,
             0,
             0);

      changes.push_back(change);
      struct kevent event;
      event_list.push_back(event);
    }

    if (!changes.size())
    {
      ::sleep(latency > MIN_SPIN_LATENCY ? latency : MIN_SPIN_LATENCY);
      continue;
    }

    const int event_num = wait_for_events(changes, event_list);
    process_events(changes, event_list, event_num);
  }
}

#endif  /* HAVE_SYS_EVENT_H */
