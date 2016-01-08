/*
 * Copyright (c) 2014-2015 Enrico M. Crisostomo
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
 * `libfswatch` monitor.
 *
 * If `HAVE_CXX_MUTEX` is defined, this header includes `<mutex>` and instances
 * of the fsw::monitor class are thread-safe.
 *
 * @copyright Copyright (c) 2014-2015 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */
#ifndef FSW__MONITOR_H
#  define FSW__MONITOR_H

#  include "filter.hpp"
#  include <vector>
#  include <string>
#  ifdef HAVE_CXX_MUTEX
#    include <mutex>
#  endif
#  include <map>
#  include "event.hpp"
#  include "../c/cmonitor.h"

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
   *   * Accessors to configuration parameters.
   *   * start() and stop() lifecycle.
   *   * Thread-safety of its API, if supported.
   *   * Event filtering.
   *   * Event notification to user-provided callback function.
   *
   * At least the following tasks must be performed to implement a monitor:
   *
   *   * Providing an implementation of the run() method.
   *   * Providing an implementation of the on_stop() method if the
   *     monitor cannot be stopped cooperatively from the run() method.
   */
  class monitor
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
     * This destructor frees the compiled regular expression of the path
     * filters, if any.
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
     * @param latency The latency value.
     */
    void set_latency(double latency);
    void set_allow_overflow(bool overflow);
    void set_recursive(bool recursive);
    void set_directory_only(bool directory_only);
    void add_filter(const monitor_filter& filter);
    void set_filters(const std::vector<monitor_filter>& filters);
    void set_follow_symlinks(bool follow);
    void *get_context() const;
    void set_context(void *context);

    /**
     * To do.
     */
    void start();

    /**
     * To do.
     */
    void stop();
    bool is_running();
    void add_event_type_filter(const fsw_event_type_filter& filter);
    void set_event_type_filters(const std::vector<fsw_event_type_filter>& filters);
    void set_watch_access(bool access);

  protected:
    bool accept_event_type(fsw_event_flag event_type) const;
    bool accept_path(const std::string& path) const;
    bool accept_path(const char *path) const;
    void notify_events(const std::vector<event>& events) const;
    void notify_overflow(const std::string& path) const;
    std::vector<fsw_event_flag> filter_flags(const event& evt) const;

    /**
     * To do.
     */
    virtual void run() = 0;

    /**
     * To do.
     */
    virtual void on_stop();

  protected:
    std::vector<std::string> paths;
    std::map<std::string, std::string> properties;
    FSW_EVENT_CALLBACK *callback;
    void *context = nullptr;
    double latency = 1.0;
    bool allow_overflow = false;
    bool recursive = false;
    bool follow_symlinks = false;
    bool directory_only = false;
    bool watch_access = false;
    bool running = false;
    bool should_stop = false;
#  ifdef HAVE_CXX_MUTEX
    std::mutex run_mutex;
#  endif

  private:
    std::vector<compiled_monitor_filter> filters;
    std::vector<fsw_event_type_filter> event_type_filters;
  };

  typedef monitor *(*FSW_FN_MONITOR_CREATOR)(std::vector<std::string> paths,
                                             FSW_EVENT_CALLBACK *callback,
                                             void *context);

  /**
   * This class maintains a register of the available monitors and let users
   * create monitors by name.  Monitors classes are required to register
   * themselves invoking the monitor_factory::register_creator and
   * monitor_factory::register_creator_by_type methods.  Registration can be
   * performed using the monitor_creator utility class and the
   * ::REGISTER_MONITOR and ::REGISTER_MONITOR_IMPL macros.
   */
  class monitor_factory
  {
  public:
    static monitor *create_monitor(fsw_monitor_type type,
                                   std::vector<std::string> paths,
                                   FSW_EVENT_CALLBACK *callback,
                                   void *context = nullptr);
    static monitor *create_monitor(const std::string& name,
                                   std::vector<std::string> paths,
                                   FSW_EVENT_CALLBACK *callback,
                                   void *context = nullptr);
    static std::vector<std::string> get_types();
    static bool exists_type(const std::string& name);
    static bool exists_type(const fsw_monitor_type& name);
    /**
     * To do.
     */
    static void register_creator(const std::string& name, FSW_FN_MONITOR_CREATOR creator);
    /**
     * To do.
     */
    static void register_creator_by_type(const fsw_monitor_type& type, FSW_FN_MONITOR_CREATOR creator);

    monitor_factory() = delete;
    monitor_factory(const monitor_factory& orig) = delete;
    monitor_factory& operator=(const monitor_factory& that) = delete;
  private:
    static std::map<std::string, FSW_FN_MONITOR_CREATOR>& creators_by_string();
    static std::map<fsw_monitor_type, FSW_FN_MONITOR_CREATOR>& creators_by_type();
  };

  /*
   * The constructor of this class perform the registration of the given
   * (name, type) pair in the monitor_factory registry.  This class is used by
   * the REGISTER_MONITOR and REGISTER_MONITOR_IMPL macros.
   */
  template<class M>
  class monitor_registrant
  {
  public:

    monitor_registrant(const std::string& name, const fsw_monitor_type& type)
    {
      FSW_FN_MONITOR_CREATOR default_creator =
        [](std::vector<std::string> paths,
           FSW_EVENT_CALLBACK *callback,
           void *context = nullptr) -> monitor *
        {
          return new M(paths, callback, context);
        };

      monitor_factory::register_creator(name, default_creator);
      monitor_factory::register_creator_by_type(type, default_creator);
    }
  };

  /**
   * This macro is used to simplify the registration process of a monitor
   * type.  Since registration of a monitor type is usually performed once, a
   * static private instance monitor_factory_registrant of the
   * monitor_registrant class is declared by this macro in the enclosing class.
   *
   * Beware that since this macro adds a private qualifier, every field
   * declared after it must be correctly qualified.
   *
   * The use of the REGISTER_MONITOR macro in a class
   * must always be matched by a corresponding use of the REGISTER_MONITOR_IMPL
   * macro in the class definition.
   *
   * To register class my_monitor with type my_type,
   * use the REGISTER_MONITOR macro as in the following example:
   *
   * [my_class.h]
   * class my_monitor
   * {
   *   REGISTER_MONITOR(my_monitor, my_monitor_type);
   *   ...
   * };
   *
   */
#  define REGISTER_MONITOR(classname, monitor_type) \
private: \
static const monitor_registrant<classname> monitor_factory_registrant;

  /**
   * This macro is used to simplify the registration process of a monitor
   * type.  Since registration of a monitor type is usually performed once, a
   * static private instance monitor_factory_registrant of the
   * monitor_registrant class is defined in the monitor class specified by
   * classname.
   *
   * A invocation of the REGISTER_MONITOR_IMPL macro must always be matched by
   * an invocation of the REGISTER_MONITOR macro in the class declaration.
   *
   * To register class my_monitor with type my_type,
   * use the REGISTER_MONITOR macro as in the following example:
   *
   * [my_class.cpp]
   *
   * REGISTER_MONITOR_IMPL(my_monitor, my_monitor_type);
   */
#  define REGISTER_MONITOR_IMPL(classname, monitor_type) \
const monitor_registrant<classname> classname::monitor_factory_registrant(#classname, monitor_type);
}

#endif  /* FSW__MONITOR_H */
