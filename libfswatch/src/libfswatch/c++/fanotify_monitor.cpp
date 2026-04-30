/*
 * Copyright (c) 2026 Enrico M. Crisostomo
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

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <libfswatch/libfswatch_config.h>

#include "fanotify_monitor.hpp"
#include "libfswatch/gettext_defs.h"
#include "libfswatch_exception.hpp"
#include "../c/libfswatch_log.h"
#include "path_utils.hpp"
#include "string/string_utils.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <limits.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifndef O_LARGEFILE
#  define O_LARGEFILE 0
#endif

#ifndef MAX_HANDLE_SZ
#  define MAX_HANDLE_SZ 128
#endif

namespace fsw
{
  namespace
  {
    constexpr size_t FANOTIFY_BUFFER_SIZE = 4096;
    constexpr size_t FILE_HANDLE_BUFFER_SIZE = sizeof(struct file_handle) + MAX_HANDLE_SZ;
    constexpr int EPOLL_EVENT_COUNT = 2;

    struct scoped_fd
    {
      int fd = -1;

      scoped_fd() = default;
      explicit scoped_fd(int fd) : fd(fd) {}
      scoped_fd(const scoped_fd&) = delete;
      scoped_fd& operator=(const scoped_fd&) = delete;

      scoped_fd(scoped_fd&& other) noexcept : fd(other.fd)
      {
        other.fd = -1;
      }

      scoped_fd& operator=(scoped_fd&& other) noexcept
      {
        if (this != &other)
        {
          reset();
          fd = other.fd;
          other.fd = -1;
        }

        return *this;
      }

      ~scoped_fd()
      {
        reset();
      }

      void reset(int new_fd = -1)
      {
        if (fd >= 0) close(fd);
        fd = new_fd;
      }

      int get() const
      {
        return fd;
      }
    };

    struct handle_buffer
    {
      std::array<unsigned char, FILE_HANDLE_BUFFER_SIZE> bytes{};

      struct file_handle *get()
      {
        return reinterpret_cast<struct file_handle *>(bytes.data());
      }
    };

    static bool string_to_bool(const std::string& value)
    {
      return value == "1" || value == "true" || value == "yes" || value == "on";
    }

    static int latency_to_timeout_ms(double latency)
    {
      if (latency <= 0) return 0;
      if (latency >= INT_MAX / 1000.0) return INT_MAX;

      return static_cast<int>(std::ceil(latency * 1000.0));
    }

    static void add_epoll_interest(int epoll_fd, int fd)
    {
      struct epoll_event event {};
      event.events = EPOLLIN;
      event.data.fd = fd;

      if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) != 0)
      {
        fsw_log_perror("epoll_ctl");
        throw libfsw_exception(_("Cannot initialize fanotify."));
      }
    }

    static void drain_eventfd(int fd)
    {
      for (;;)
      {
        uint64_t value = 0;
        ssize_t read_count = read(fd, &value, sizeof(value));

        if (read_count == static_cast<ssize_t>(sizeof(value))) continue;
        if (read_count == -1 && errno == EINTR) continue;
        if (read_count == -1 && errno == EAGAIN) break;
        if (read_count == -1)
        {
          fsw_log_perror("read");
          break;
        }

        break;
      }
    }

    static std::string make_handle_key(const void *fsid,
                                       size_t fsid_size,
                                       const struct file_handle *handle)
    {
      std::string key;
      key.append(reinterpret_cast<const char *>(fsid), fsid_size);
      key.append(reinterpret_cast<const char *>(&handle->handle_type), sizeof(handle->handle_type));
      key.append(reinterpret_cast<const char *>(&handle->handle_bytes), sizeof(handle->handle_bytes));
      key.append(reinterpret_cast<const char *>(handle->f_handle), handle->handle_bytes);

      return key;
    }

    static bool get_path_handle(const std::filesystem::path& path,
                                std::string& key)
    {
      struct statfs fs_stats{};
      if (statfs(path.c_str(), &fs_stats) != 0)
      {
        fsw_log_perror("statfs");
        return false;
      }

      handle_buffer buffer;
      struct file_handle *handle = buffer.get();
      handle->handle_bytes = MAX_HANDLE_SZ;
      int mount_id = 0;

      if (name_to_handle_at(AT_FDCWD, path.c_str(), handle, &mount_id, 0) != 0)
      {
        fsw_log_perror("name_to_handle_at");
        return false;
      }

      key = make_handle_key(&fs_stats.f_fsid, sizeof(fs_stats.f_fsid), handle);
      return true;
    }

    static std::vector<fsw_event_flag> flags_from_mask(uint64_t mask)
    {
      std::vector<fsw_event_flag> flags;

      if (mask & FAN_ONDIR) flags.push_back(fsw_event_flag::IsDir);
      if (mask & FAN_CREATE) flags.push_back(fsw_event_flag::Created);
      if (mask & FAN_MODIFY) flags.push_back(fsw_event_flag::Updated);
      if (mask & FAN_CLOSE_WRITE) flags.push_back(fsw_event_flag::CloseWrite);
      if (mask & FAN_ATTRIB) flags.push_back(fsw_event_flag::AttributeModified);
      if (mask & FAN_DELETE) flags.push_back(fsw_event_flag::Removed);
      if (mask & FAN_DELETE_SELF) flags.push_back(fsw_event_flag::Removed);
      if (mask & FAN_MOVED_FROM)
      {
        flags.push_back(fsw_event_flag::Removed);
        flags.push_back(fsw_event_flag::MovedFrom);
      }
      if (mask & FAN_MOVED_TO)
      {
        flags.push_back(fsw_event_flag::Created);
        flags.push_back(fsw_event_flag::MovedTo);
      }
      if (mask & FAN_MOVE_SELF) flags.push_back(fsw_event_flag::Renamed);
      if (mask & FAN_ACCESS) flags.push_back(fsw_event_flag::PlatformSpecific);
      if (mask & FAN_OPEN) flags.push_back(fsw_event_flag::PlatformSpecific);
      if (mask & FAN_CLOSE_NOWRITE) flags.push_back(fsw_event_flag::PlatformSpecific);

      return flags;
    }

    static bool is_directory_event(uint64_t mask)
    {
      return (mask & FAN_ONDIR) != 0;
    }
  }

  struct fanotify_monitor_impl
  {
    scoped_fd fanotify_fd;
    scoped_fd epoll_fd;
    scoped_fd wake_fd;
    std::vector<event> events;
    std::vector<int> pidfds_to_close;
    std::unordered_set<std::string> watched_paths;
    std::unordered_map<std::string, std::string> handle_to_path;
    std::vector<std::string> paths_to_rescan;
    std::vector<std::string> paths_to_fire_create;
    process_id_kind process_kind = process_id_kind::pid;
    bool report_pidfd = false;
    bool initialized = false;
    time_t curr_time = 0;
  };

  fanotify_monitor::fanotify_monitor(std::vector<std::string> paths_to_monitor,
                                     FSW_EVENT_CALLBACK *callback,
                                     void *context) :
    monitor(std::move(paths_to_monitor), callback, context),
    impl(std::make_unique<fanotify_monitor_impl>())
  {
  }

  fanotify_monitor::~fanotify_monitor() = default;

  void fanotify_monitor::initialize()
  {
    if (impl->initialized) return;

    const std::string process_id_property = get_property(PROCESS_ID_PROPERTY);
    if (!process_id_property.empty())
    {
      if (process_id_property == "pid")
      {
        impl->process_kind = process_id_kind::pid;
      }
      else if (process_id_property == "tid")
      {
#if defined(FAN_REPORT_TID)
        impl->process_kind = process_id_kind::tid;
#else
        throw libfsw_exception(_("fanotify TID reporting is not supported by the build headers."));
#endif
      }
      else
      {
        throw libfsw_exception(_("Invalid fanotify.process-id value."));
      }
    }

    impl->report_pidfd = string_to_bool(get_property(REPORT_PIDFD_PROPERTY));

    if (impl->report_pidfd && impl->process_kind == process_id_kind::tid)
      throw libfsw_exception(_("fanotify.report-pidfd=true is incompatible with fanotify.process-id=tid."));

    unsigned int init_flags = FAN_CLASS_NOTIF | FAN_CLOEXEC | FAN_NONBLOCK | FAN_REPORT_DFID_NAME;

    const bool unlimited_queue = string_to_bool(get_property(UNLIMITED_QUEUE_PROPERTY));
    const bool unlimited_marks = string_to_bool(get_property(UNLIMITED_MARKS_PROPERTY));

    if (unlimited_queue)
    {
#if defined(FAN_UNLIMITED_QUEUE)
      init_flags |= FAN_UNLIMITED_QUEUE;
#else
      throw libfsw_exception(std::string(UNLIMITED_QUEUE_PROPERTY) +
                             "=true requires FAN_UNLIMITED_QUEUE support in the build headers.");
#endif
    }

    if (unlimited_marks)
    {
#if defined(FAN_UNLIMITED_MARKS)
      init_flags |= FAN_UNLIMITED_MARKS;
#else
      throw libfsw_exception(std::string(UNLIMITED_MARKS_PROPERTY) +
                             "=true requires FAN_UNLIMITED_MARKS support in the build headers.");
#endif
    }

#if defined(FAN_REPORT_TID)
    if (impl->process_kind == process_id_kind::tid) init_flags |= FAN_REPORT_TID;
#endif

    if (impl->report_pidfd)
    {
#if defined(FAN_REPORT_PIDFD)
      init_flags |= FAN_REPORT_PIDFD;
#else
      throw libfsw_exception(_("fanotify pidfd reporting is not supported by the build headers."));
#endif
    }

    scoped_fd fanotify_fd(fanotify_init(init_flags, O_RDONLY | O_CLOEXEC | O_LARGEFILE));
    if (fanotify_fd.get() < 0)
    {
      const int fanotify_errno = errno;
      fsw_log_perror("fanotify_init");
      if (fanotify_errno == EPERM && (unlimited_queue || unlimited_marks))
      {
        throw libfsw_exception("fanotify.unlimited-queue=true or fanotify.unlimited-marks=true requires CAP_SYS_ADMIN.");
      }

      throw libfsw_exception(_("Cannot initialize fanotify."));
    }

    scoped_fd wake_fd(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK));
    if (wake_fd.get() < 0)
    {
      fsw_log_perror("eventfd");
      throw libfsw_exception(_("Cannot initialize fanotify."));
    }

    scoped_fd epoll_fd(epoll_create1(EPOLL_CLOEXEC));
    if (epoll_fd.get() < 0)
    {
      fsw_log_perror("epoll_create1");
      throw libfsw_exception(_("Cannot initialize fanotify."));
    }

    add_epoll_interest(epoll_fd.get(), fanotify_fd.get());
    add_epoll_interest(epoll_fd.get(), wake_fd.get());

    impl->fanotify_fd = std::move(fanotify_fd);
    impl->wake_fd = std::move(wake_fd);
    impl->epoll_fd = std::move(epoll_fd);
    impl->initialized = true;
  }

  bool fanotify_monitor::is_watched(const std::string& path) const
  {
    return impl->watched_paths.find(path) != impl->watched_paths.end();
  }

  bool fanotify_monitor::add_mark(const std::filesystem::path& path)
  {
    uint64_t event_mask =
      FAN_MODIFY | FAN_CLOSE_WRITE | FAN_ATTRIB |
      FAN_CREATE | FAN_DELETE | FAN_DELETE_SELF |
      FAN_MOVED_FROM | FAN_MOVED_TO | FAN_MOVE_SELF |
      FAN_ONDIR | FAN_EVENT_ON_CHILD;

    if (watch_access)
    {
      event_mask |= FAN_ACCESS | FAN_OPEN | FAN_CLOSE_NOWRITE;
    }

    if (fanotify_mark(impl->fanotify_fd.get(),
                      FAN_MARK_ADD,
                      event_mask,
                      AT_FDCWD,
                      path.c_str()) != 0)
    {
      fsw_log_perror("fanotify_mark");
      return false;
    }

    impl->watched_paths.insert(path.string());

    std::string handle_key;
    if (get_path_handle(path, handle_key))
    {
      impl->handle_to_path[handle_key] = path.string();
    }

    FSW_ELOGF(_("fanotify added: %s\n"), path.c_str());
    return true;
  }

  void fanotify_monitor::scan(const std::filesystem::path& path, const bool is_root_path)
  {
    try
    {
      auto status = std::filesystem::symlink_status(path);

      if (!std::filesystem::exists(status)) return;

      if (follow_symlinks && std::filesystem::is_symlink(status))
      {
        scan(std::filesystem::read_symlink(path), is_root_path);
        return;
      }

      const bool is_dir = std::filesystem::is_directory(status);
      if (should_prune_path(path.string(), is_dir, is_root_path)) return;
      if (!is_dir && !is_root_path) return;
      if (!is_dir && directory_only) return;
      if (is_watched(path.string())) return;
      if (!add_mark(path)) return;
      if (!recursive || !is_dir) return;

      const auto entries = get_subdirectories(path);
      for (const auto& entry : entries)
      {
        scan(entry, false);
      }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
      FSW_ELOGF(_("Filesystem error: %s"), e.what());
    }
  }

  void fanotify_monitor::scan_root_paths()
  {
    for (const std::string& path : paths)
    {
      if (!is_watched(path)) scan(path, true);
    }
  }

  void fanotify_monitor::process_pending_paths()
  {
    for (const auto& path : impl->paths_to_rescan)
    {
      scan(path);
    }

    impl->paths_to_rescan.clear();
  }

  void fanotify_monitor::process_synthetic_events()
  {
    for (const std::string& path : impl->paths_to_fire_create)
    {
      FSW_ELOGF(_("Synthetic event: processing directory: %s\n"), path.c_str());

      const auto entries = get_directory_entries(path);

      for (const auto& entry : entries)
      {
        std::vector<fsw_event_flag> flags{fsw_event_flag::Created};

        try
        {
          if (entry.is_directory()) flags.push_back(fsw_event_flag::IsDir);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
          FSW_ELOGF(_("Filesystem error: %s"), e.what());
        }

        impl->events.emplace_back(entry.path().string(), impl->curr_time, flags);
      }
    }

    impl->paths_to_fire_create.clear();
  }

  void fanotify_monitor::process_events(char *buffer, ssize_t length)
  {
    time(&impl->curr_time);

    for (auto *metadata = reinterpret_cast<struct fanotify_event_metadata *>(buffer);
         FAN_EVENT_OK(metadata, length);
         metadata = FAN_EVENT_NEXT(metadata, length))
    {
      if (metadata->vers != FANOTIFY_METADATA_VERSION)
        throw libfsw_exception(_("fanotify metadata version mismatch."));

      if (metadata->mask & FAN_Q_OVERFLOW)
      {
        notify_overflow("");
        continue;
      }

      std::string path;
      int pidfd = -1;
      bool has_pidfd = false;

      auto *info = reinterpret_cast<struct fanotify_event_info_header *>(
        reinterpret_cast<char *>(metadata) + metadata->metadata_len);
      char *info_end = reinterpret_cast<char *>(metadata) + metadata->event_len;

      while (reinterpret_cast<char *>(info) + sizeof(*info) <= info_end &&
             info->len >= sizeof(*info) &&
             reinterpret_cast<char *>(info) + info->len <= info_end)
      {
        switch (info->info_type)
        {
        case FAN_EVENT_INFO_TYPE_DFID_NAME:
        case FAN_EVENT_INFO_TYPE_DFID:
        case FAN_EVENT_INFO_TYPE_FID:
        {
          auto *fid = reinterpret_cast<struct fanotify_event_info_fid *>(info);
          auto *file_handle = reinterpret_cast<struct file_handle *>(fid->handle);
          std::string handle_key = make_handle_key(&fid->fsid, sizeof(fid->fsid), file_handle);

          auto cached_path = impl->handle_to_path.find(handle_key);
          if (cached_path == impl->handle_to_path.end()) break;

          path = cached_path->second;

          if (info->info_type == FAN_EVENT_INFO_TYPE_DFID_NAME)
          {
            const char *name = reinterpret_cast<const char *>(
              file_handle->f_handle + file_handle->handle_bytes);

            if (name[0] != '\0' && std::strcmp(name, ".") != 0)
            {
              path = (std::filesystem::path(path) / name).string();
            }
          }

          break;
        }
#if defined(FAN_EVENT_INFO_TYPE_PIDFD)
        case FAN_EVENT_INFO_TYPE_PIDFD:
        {
          auto *pidfd_info = reinterpret_cast<struct fanotify_event_info_pidfd *>(info);
          if (pidfd_info->pidfd >= 0)
          {
            pidfd = pidfd_info->pidfd;
            has_pidfd = true;
            impl->pidfds_to_close.push_back(pidfd);
          }
          break;
        }
#endif
        default:
          break;
        }

        info = reinterpret_cast<struct fanotify_event_info_header *>(
          reinterpret_cast<char *>(info) + info->len);
      }

      if (path.empty())
      {
        FSW_ELOG(_("fanotify event path could not be reconstructed.\n"));
        continue;
      }

      std::vector<fsw_event_flag> flags = flags_from_mask(metadata->mask);
      if (flags.empty()) continue;

      process_metadata process;
      if (metadata->pid > 0)
      {
        process.kind = impl->process_kind;
        process.id = metadata->pid;
      }
      process.pidfd = pidfd;
      process.has_pidfd = has_pidfd;

      if (recursive &&
          is_directory_event(metadata->mask) &&
          (metadata->mask & (FAN_CREATE | FAN_MOVED_TO)))
      {
        impl->paths_to_rescan.push_back(path);
        impl->paths_to_fire_create.push_back(path);
      }

      impl->events.emplace_back(path, impl->curr_time, flags, 0, process);
    }
  }

  void fanotify_monitor::notify_and_clear_events()
  {
    if (!impl->events.empty())
    {
      notify_events(impl->events);
      impl->events.clear();
    }

    for (int pidfd : impl->pidfds_to_close)
    {
      close(pidfd);
    }

    impl->pidfds_to_close.clear();
  }

  void fanotify_monitor::on_stop()
  {
    if (!impl || impl->wake_fd.get() < 0) return;

    uint64_t value = 1;
    for (;;)
    {
      ssize_t write_count = write(impl->wake_fd.get(), &value, sizeof(value));

      if (write_count == static_cast<ssize_t>(sizeof(value))) break;
      if (write_count == -1 && errno == EINTR) continue;
      if (write_count == -1 && errno == EAGAIN) break;

      fsw_log_perror("write");
      break;
    }
  }

  void fanotify_monitor::run()
  {
    initialize();

    const int timeout_ms = latency_to_timeout_ms(this->latency);
    std::array<char, FANOTIFY_BUFFER_SIZE> buffer{};
    std::array<struct epoll_event, EPOLL_EVENT_COUNT> epoll_events{};

    for (;;)
    {
      std::unique_lock<std::mutex> run_guard(run_mutex);
      if (should_stop) break;
      run_guard.unlock();

      process_pending_paths();
      scan_root_paths();

      if (impl->watched_paths.empty())
      {
        int rv = epoll_wait(impl->epoll_fd.get(),
                            epoll_events.data(),
                            epoll_events.size(),
                            timeout_ms);

        if (rv == -1)
        {
          if (errno == EINTR) continue;
          fsw_log_perror("epoll_wait");
          continue;
        }

        if (rv > 0)
        {
          for (int i = 0; i < rv; ++i)
          {
            if (epoll_events[i].data.fd == impl->wake_fd.get())
            {
              drain_eventfd(impl->wake_fd.get());
              return;
            }
          }
        }

        continue;
      }

      int rv = epoll_wait(impl->epoll_fd.get(),
                          epoll_events.data(),
                          epoll_events.size(),
                          timeout_ms);

      if (rv == -1)
      {
        if (errno == EINTR) continue;
        fsw_log_perror("epoll_wait");
        continue;
      }

      if (rv == 0) continue;

      bool wake_requested = false;

      for (int i = 0; i < rv; ++i)
      {
        if (epoll_events[i].data.fd == impl->wake_fd.get())
        {
          drain_eventfd(impl->wake_fd.get());
          wake_requested = true;
          continue;
        }

        if (epoll_events[i].data.fd != impl->fanotify_fd.get())
        {
          continue;
        }

        for (;;)
        {
          ssize_t record_num = read(impl->fanotify_fd.get(),
                                    buffer.data(),
                                    buffer.size());

          if (record_num == -1)
          {
            if (errno == EAGAIN) break;
            if (errno == EINTR) continue;
            fsw_log_perror("read");
            throw libfsw_exception(_("read() on fanotify descriptor returned -1."));
          }

          if (record_num == 0) break;

          process_events(buffer.data(), record_num);
        }
      }

      notify_and_clear_events();
      if (wake_requested) break;

      process_pending_paths();
      process_synthetic_events();
      notify_and_clear_events();
    }
  }
}
