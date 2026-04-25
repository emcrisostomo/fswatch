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
/**
 * @file
 * @brief Header of the fsw::fanotify_monitor class.
 */

#ifndef FSW_FANOTIFY_MONITOR_H
#  define FSW_FANOTIFY_MONITOR_H

#  include "monitor.hpp"
#  include <string>
#  include <vector>
#  include <filesystem>
#  include <memory>
#  include <sys/types.h>

namespace fsw
{
  struct fanotify_monitor_impl;

  /**
   * @brief Linux fanotify monitor.
   *
   * This monitor is backed by the Linux fanotify API and is available only on
   * systems whose headers and C library expose the required fanotify features.
   */
  class fanotify_monitor : public monitor
  {
  public:
    static constexpr const char *PROCESS_ID_PROPERTY = "fanotify.process-id";
    static constexpr const char *REPORT_PIDFD_PROPERTY = "fanotify.report-pidfd";
    static constexpr const char *UNLIMITED_QUEUE_PROPERTY = "fanotify.unlimited-queue";
    static constexpr const char *UNLIMITED_MARKS_PROPERTY = "fanotify.unlimited-marks";

    fanotify_monitor(std::vector<std::string> paths,
                     FSW_EVENT_CALLBACK *callback,
                     void *context = nullptr);

    ~fanotify_monitor() override;

  protected:
    void run() override;
    void on_stop() override;

  private:
    fanotify_monitor(const fanotify_monitor& orig) = delete;
    fanotify_monitor& operator=(const fanotify_monitor& that) = delete;

    void initialize();
    void scan_root_paths();
    void scan(const std::filesystem::path& path, bool accept_non_dirs = true);
    bool add_mark(const std::filesystem::path& path);
    bool is_watched(const std::string& path) const;
    void process_pending_paths();
    void process_synthetic_events();
    void process_events(char *buffer, ssize_t length);
    void notify_and_clear_events();

    std::unique_ptr<fanotify_monitor_impl> impl;
  };
}

#endif  /* FSW_FANOTIFY_MONITOR_H */
