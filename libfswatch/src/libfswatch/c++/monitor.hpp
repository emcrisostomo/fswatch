/*
 * Copyright (c) 2014-2021 Enrico M. Crisostomo
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
 * @brief Header of the fsw::monitor class.
 *
 * This header file defines the fsw::monitor class, the base type of a
 * `libfswatch` monitor and fundamental type of the C++ API.
 *
 * If `HAVE_CXX_MUTEX` is defined, this header includes `<mutex>`.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */
#ifndef FSW__MONITOR_H
#  define FSW__MONITOR_H

#  include "libfswatch/libfswatch_config.h"
#  include "filter.hpp"
#  include <vector>
#  include <string>
#  ifdef HAVE_CXX_MUTEX
#    include <mutex>
#  endif
#  include <atomic>
#  include <chrono>
#  include <map>
#  include "event.hpp"
#  include "cmonitor.h"
#  include "cxxfswatch_export.h"

/**
 * @brief Main namespace of `libfswatch`.
 */
namespace fsw
{
  /**
   * @brief Function definition of an event callback.
   *
   * The event callback is a user-supplied function that is invoked by the
   * monitor when an event is detected.  The following parameters are passed to
   * the callback:
   *
   *   * A reference to the vector of events.
   *   * A pointer to the _context data_ set by the caller.
   */
  typedef void FSW_EVENT_CALLBACK(const std::vector<event>&, void *);

  struct compiled_monitor_filter;

  /**
   * @brief Base class of all monitors.
   *
   * The fsw::monitor class is the base class of all monitors.  This class
   * encapsulates the common functionality of a monitor:
   *
   *   - Accessors to configuration parameters.
   *   - start() and stop() lifecycle.
   *   - Event filtering.
   *   - Event notification to user-provided callback function.
   *
   * Since some methods are designed to be called from different threads, this
   * class provides an internal mutex (monitor::run_mutex) that implementors
   * should lock on when accessing shared state.  The mutex is available only
   * when `HAVE_CXX_MUTEX` is defined.
   *
   * At least the following tasks must be performed to implement a monitor:
   *
   *   - Providing an implementation of the run() method.
   *   - Providing an implementation of the on_stop() method if the
   *     monitor cannot be stopped cooperatively from the run() method.
   *
   * A basic monitor needs to implement the run() method, whose skeleton is
   * often similar to the following:
   *
   *     void run()
   *     {
   *       initialize_api();
   *
   *       for (;;)
   *       {
   *         #ifdef HAVE_CXX_MUTEX
   *           unique_lock<mutex> run_guard(run_mutex);
   *           if (should_stop) break;
   *           run_guard.unlock();
   *         #endif
   *
   *         scan_paths();
   *         wait_for_events();
   *
   *         vector<change_events> evts = get_changes();
   *         vector<event> events;
   *
   *         for (auto & evt : evts)
   *         {
   *           if (accept(evt.get_path))
   *           {
   *             events.push_back({event from evt});
   *           }
   *         }
   *
   *         if (events.size()) notify_events(events);
   *       }
   *
   *       terminate_api();
   *     }
   *
   * Despite being a minimal implementation, it performs all the tasks commonly
   * performed by a monitor:
   *
   *   - It initializes the API it uses to detect file system change events.
   *
   *   - It enters a loop, often infinite, where change events are waited for.
   *
   *   - If `HAVE_CXX_MUTEX` is defined, it locks on monitor::run_mutex to
   *     check whether monitor::should_stop is set to @c true.  If it is, the
   *     monitor breaks the loop to return from run() as soon as possible.
   *
   *   - It scans the paths that must be observed: this step might be necessary
   *     for example because some path may not have existed during the previous
   *     iteration of the loop, or because some API may require the user to
   *     re-register a watch on a path after events are retrieved.
   *
   *   - Events are waited for and the wait should respect the specified
   *     _latency_.
   *
   *   - Events are _filtered_ to exclude those referring to paths that do not
   *     satisfy the configured filters.
   *
   *   - The notify_events() method is called to filter the event types and
   *     notify the caller.
   */
  class CXXFSWATCH_EXPORT monitor
  {
  public:
    /**
     * @brief Constructs a monitor watching the specified @p paths.
     *
     * The monitor will notify change events to the specified @p callback,
     * passing it the pointer to the specified @p context.
     *
     * @param paths The list of paths to watch.
     * @param callback The callback to which change events will be notified.
     *                 The callback cannot be null, otherwise a libfsw_exception
     *                 will be thrown.
     * @param context An optional pointer to context data.  The monitor stores a
     *                copy of this pointer to pass it to the @p callback.
     */
    monitor(std::vector<std::string> paths,
            FSW_EVENT_CALLBACK *callback,
            void *context = nullptr);

    /**
     * @brief Destructs a monitor instance.
     *
     * This destructor performs the following operations:
     *
     *   - Stops the monitor.
     *
     *   - Frees the compiled regular expression of the path filters, if any.
     *
     * @warning Destroying a monitor in the _running_ state results in undefined
     * behaviour.
     *
     * @see stop()
     */
    virtual ~monitor();

    /**
     * @brief This class is not copy constructible.
     */
    monitor(const monitor& orig) = delete;

    /**
     * @brief This class is not copy assignable.
     */
    monitor& operator=(const monitor& that) = delete;

    /**
     * @brief Sets a custom property.
     *
     * This method sets the custom property @p name to @p value.
     *
     * @param name The name of the property.
     * @param value The value of the property.
     */
    void set_property(const std::string& name, const std::string& value);

    /**
     * @brief Sets the custom properties.
     *
     * This method _replaces_ all the existing properties using the pairs
     * contained into @p options.
     *
     * @param options The map containing the properties to set.
     */
    void set_properties(const std::map<std::string, std::string> options);

    /**
     * @brief Gets the value of a property.
     *
     * This method gets the value of the property @p name.  If the property
     * @p name is not set, this method returns an empty string.
     *
     * @param name The name of the property.
     * @return The value of the property.
     */
    std::string get_property(std::string name);

    /**
     * @brief Sets the latency.
     *
     * This method sets the _latency_ of the monitor to @p latency.  The latency
     * is a positive number that indicates to a monitor implementation how often
     * events must be retrieved or waited for: the shortest the latency, the
     * quicker events are processed.
     *
     * @warning The behaviour associated with this flag depends on the
     * implementation.
     *
     * @param latency The latency value.
     */
    void set_latency(double latency);

    /**
     * @brief Sets the _fire idle event_ flag.
     *
     * When @c true, the _fire idle event_ flag instructs the monitor to fire a
     * fake event at the event of an _idle_ cycle.  An idle cycle is a period of
     * time whose length is 110% of the monitor::latency where no change events
     * were detected.
     *
     * @param fire_idle_event @c true if idle events should be fired, @c false
     * otherwise.
     */
    void set_fire_idle_event(bool fire_idle_event);

    /**
     * @brief Notify buffer overflows as change events.
     *
     * If this flag is set, the monitor will report a monitor buffer overflow as
     * a change event of type fsw_event_flag::Overflow.
     *
     * @warning The behaviour associated with this flag depends on the
     * implementation.
     *
     * @param overflow @c true if overflow should be notified, @c false
     * otherwise.
     */
    void set_allow_overflow(bool overflow);

    /**
     * @brief Recursively scan subdirectories.
     *
     * This function sets the recursive flag of the monitor to indicate whether
     * the monitor should recursively observe the contents of directories.  The
     * behaviour associated with this flag is an implementation-specific detail.
     * This class only stores the value of the flag.
     *
     * @warning The behaviour associated with this flag depends on the
     * implementation.
     *
     * @param recursive @c true if directories should be recursively, @c false
     * otherwise.
     */
    void set_recursive(bool recursive);

    /**
     * @brief Watch directories only.
     *
     * This function sets the directory only flag to the specified value.  If
     * this flag is set, then the monitor will only watch directories during a
     * recursive scan.  This functionality is only supported by monitors whose
     * backend fires change events on a directory when one its children is
     * changed.  If a monitor backend does not support this functionality, the
     * flag is ignored.
     *
     * @warning The behaviour associated with this flag depends on the
     * implementation.
     *
     * @param directory_only @c true if only directories should be watched,
     * @c flase otherwise.
     */
    void set_directory_only(bool directory_only);

    /**
     * @brief Add a path filter.
     *
     * This function adds a monitor_filter instance instance to the filter list.
     *
     * @param filter The filter to add.
     */
    void add_filter(const monitor_filter& filter);

    /**
     * @brief Set the path filters.
     *
     * This function sets the list of path filters, substituting existing
     * filters if any.
     *
     * @param filters The filters to set.
     */
    void set_filters(const std::vector<monitor_filter>& filters);

    /**
     * @brief Follow symlinks.
     *
     * This function sets the follow_symlinks flag of the monitor to indicate
     * whether the monitor should follow symbolic links or observe the links
     * themselves.
     *
     * @warning The behaviour associated with this flag depends on the
     * implementation.
     *
     * @param follow @c true if symbolic links should be followed, @c false
     * otherwise.
     */
    void set_follow_symlinks(bool follow);

    /**
     * @brief Get the pointer to the context data.
     *
     * This function gets the pointer to the context data that is passed to the
     * callback by the monitor.
     *
     * @return The pointer to the context data.
     */
    void *get_context() const;

    /**
     * @brief Set the context data.
     *
     * This function sets the pointer to the _context data_.  The context data
     * is opaque data that the monitor passes to the event callback.
     *
     * @warning The monitor stores the pointer to the context data throughout
     * its life.  The caller must ensure it points to valid data until the
     * monitor is running.
     *
     * @param context The pointer to the context data.
     */
    void set_context(void *context);

    /**
     * @brief Set the bubble events flag.
     *
     * This function sets the bubble events flags, instructing the monitor to
     * consolidate the event flags for all events with the same time and path
     * received in the same batch.
     *
     * @param bubble_events The bubble events flag.
     */
    void set_bubble_events(bool bubble_events);

    /**
     * @brief Start the monitor.
     *
     * The monitor status is marked as _running_ and it starts watching for
     * change events.  This function performs the following tasks:
     *
     *   * Atomically marks the thread state as _running_, locking on
     *     monitor::run_mutex.
     *   * Calls the run() function: the monitor::run_mutex is **not** locked
     *     during this call.
     *   * When run() returns, it atomically marks the thread state as
     *     _stopped_, locking on monitor::run_mutex.
     *
     * This call does _not_ return until the monitor is stopped and events are
     * notified from its thread.
     *
     * State changes are performed thread-safely locking on monitor::run_mutex.
     *
     * @see run()
     * @see stop()
     */
    void start();

    /**
     * @brief Stop the monitor.
     *
     * This function asks the monitor to stop.  Since start() is designed to
     * execute the monitoring loop in its thread and to not return until the
     * monitor is stopped, stop() is designed to be called from another thread.
     * stop() is a cooperative signal that must be handled in an
     * implementation-specific way in the run() function.
     *
     * State changes are performed thread-safely locking on monitor::run_mutex.
     *
     * @see run()
     * @see start()
     */
    void stop();

    /**
     * @brief Check whether the monitor is running.
     *
     * State is checked thread-safely locking on monitor::run_mutex.
     *
     * @return @c true if the monitor is running, @c false otherwise.
     */
    bool is_running();

    /**
     * @brief Add an event type filter.
     *
     * Adds a fsw_event_type_filter instance to filter events by _type_.
     *
     * @param filter The event type filter to add.
     */
    void add_event_type_filter(const fsw_event_type_filter& filter);

    /**
     * @brief Set the event type filters.
     *
     * This function sets the list of event type filters, substituting existing
     * filters if any.
     *
     * @param filters The filters to set.
     */
    void set_event_type_filters(
      const std::vector<fsw_event_type_filter>& filters);

    /**
     * @brief Monitor file access.
     *
     * @warning The ability of monitoring file access depends on a monitor
     * implementation.
     */
    void set_watch_access(bool access);

  protected:
    /**
     * @brief Check whether an event should be accepted.
     *
     * This function checks @p event_type against the event type filters of the
     * monitor to determine whether it should be _accepted_.
     *
     * @param event_type The event type to check.
     * @return @c true if the event is accepted, @c false otherwise.
     */
    bool accept_event_type(fsw_event_flag event_type) const;

    /**
     * @brief Check whether a path should be accepted.
     *
     * This function checks @p path against the path filters of the monitor to
     * determine whether it should be _accepted_.
     *
     * @param event_type The path to check.
     * @return @c true if the path is accepted, @c false otherwise.
     */
    bool accept_path(const std::string& path) const;

    /**
     * @brief Notify change events.
     *
     * This function notifies change events using the provided callback.
     *
     * @see monitor()
     */
    void notify_events(const std::vector<event>& events) const;

    /**
     * @brief Notify an overflow event.
     *
     * This function notifies an overflow event using the provided callback.
     *
     * @warning Experiencing an overflow and the ability to notify it is an
     * implementation-defined behaviour.
     *
     * @see monitor()
     */
    void notify_overflow(const std::string& path) const;

    /**
     * @brief Filter event types.
     *
     * This function filters the event types of an event leaving only the types
     * allowed by the configured filters.
     *
     * @param evt The event whose types must be filtered.
     * @return A vector containing the acceptable events.
     */
    std::vector<fsw_event_flag> filter_flags(const event& evt) const;

    /**
     * @brief Execute monitor loop.
     *
     * This function implements the monitor event watching logic.  This function
     * is called from start() and it is executed on its thread.  This function
     * should _block_ until the monitoring loop terminates: when it returns, the
     * monitor is marked as stopped.
     *
     * This function should cooperatively check the monitor::should_stop field
     * locking monitor::run_mutex and return if set to @c true.
     *
     * @see start()
     * @see stop()
     */
    virtual void run() = 0;

    /**
     * @brief Execute an implementation-specific stop handler.
     *
     * This function is executed by the stop() method, after requesting the
     * monitor to stop.  This handler is required if the thread running run() is
     * not able to preemptively stop its execution by checking the
     * monitor::should_stop flag.
     *
     * @see stop()
     */
    virtual void on_stop();

  protected:
    /**
     * @brief List of paths to watch.
     *
     * @see monitor::monitor()
     */
    std::vector<std::string> paths;

    /**
     * @brief Map of custom properties.
     *
     * @see monitor::set_property()
     * @see monitor::set_properties()
     */
    std::map<std::string, std::string> properties;

    /**
     * @brief Callback to which change events should be notified.
     *
     * @see monitor::monitor()
     */
    FSW_EVENT_CALLBACK *callback;

    /**
     * @brief Pointer to context data that will be passed to the monitor::callback.
     */
    void *context = nullptr;

    /**
     * @brief Latency of the monitor.
     */
    double latency = 1.0;

    /**
     * @brief If @c true, the monitor will notify an event when idle.
     *
     * An idle cycle is long as 110% of the monitor::latency value.
     */
    bool fire_idle_event = false;

    /**
     * @brief If @c true, queue overflow events will be notified to the caller,
     * otherwise the monitor will throw a libfsw_exception.
     */
    bool allow_overflow = false;

    /**
     * @brief If @c true, directories will be scanned recursively.
     */
    bool recursive = false;

    /**
     * @brief If @c true, symbolic links are followed.
     */
    bool follow_symlinks = false;

    /**
     * @brief Flag indicating whether only directories should be monitored.
     */
    bool directory_only = false;

    /**
     * @brief Flag indicating whether file access should be watched.
     */
    bool watch_access = false;

    /**
     * @brief Flag indicating whether the monitor is in the running state.
     */
    bool running = false;

    /**
     * @brief Flag indicating whether the monitor should preemptively stop.
     */
    bool should_stop = false;

    /**
     * @brief Bubble events by joining flags received for the same (time, path)
     * pair.
     */
    bool bubble_events = false;

#  ifdef HAVE_CXX_MUTEX
    /**
     * @brief Mutex used to serialize access to the monitor state from multiple
     * threads.
     */
    mutable std::mutex run_mutex;

    /**
     * @brief Mutex used to serialize access to the notify_events() method.
     */
    mutable std::mutex notify_mutex;
#  endif

  private:
    std::chrono::milliseconds get_latency_ms() const;
    std::vector<compiled_monitor_filter> filters;
    std::vector<fsw_event_type_filter> event_type_filters;

#ifdef HAVE_CXX_MUTEX
# ifdef HAVE_CXX_ATOMIC
#   define HAVE_INACTIVITY_CALLBACK
    static void inactivity_callback(monitor *mon);
    mutable std::atomic<std::chrono::milliseconds> last_notification;
# endif
#endif
  };
}

#endif  /* FSW__MONITOR_H */
