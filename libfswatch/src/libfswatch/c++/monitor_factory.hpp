/*
 * Copyright (c) 2014-2018 Enrico M. Crisostomo
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
 * @brief Header of the fsw::monitor_factory class.
 *
 * This header file defines the fsw::monitor_factory class, the base type of a
 * `libfswatch` monitor factory.
 *
 * @copyright Copyright (c) 2014-2018 Enrico M. Crisostomo
 * @license GNU General Public License v. 3.0
 * @author Enrico M. Crisostomo
 * @version 1.8.0
 */
#ifndef FSW__MONITOR_FACTORY_H
#  define FSW__MONITOR_FACTORY_H

#include "monitor.hpp"
#include "libfswatch_set.hpp"

namespace fsw
{
  /**
   * @brief Object factory class for fsw::monitor instances.
   *
   * Since multiple monitor implementations exist and the caller potentially
   * ignores which monitors will be available at run time, there must exist a
   * way to query the API for the list of available monitor and request a
   * particular instance.  The fsw::monitor_factory is an object factory class
   * that provides basic monitor _registration_ and _discovery_ functionality:
   * API clients can query the monitor registry to get a list of available
   * monitors and get an instance of a monitor either by _type_ or by _name_.
   *
   * In order for monitor types to be visible to the factory they have to be
   * _registered_.  Currently, monitor implementations are registered at compile
   * time.
   *
   * The same monitor type cannot be used to register multiple monitor
   * implementations.  No checks are in place to detect this situation and the
   * registration will succeed; however, the registration process of multiple
   * monitor implementations for the same monitor type is _not_ deterministic.
   */
  class monitor_factory
  {
  public:
    /**
     * @brief Creates a monitor of the specified @p type.
     *
     * The other parameters are forwarded to the fsw::monitor() constructor.
     *
     * @param type The monitor type.
     * @param paths The paths to watch.
     * @param callback The callback to invoke during the notification of a
     * change event.
     * @return The newly created monitor.
     * @throw libfsw_exception if a monitor of the specified @p type cannot be
     * found.
     * @see fsw::monitor()
     */
    static monitor *create_monitor(fsw_monitor_type type,
                                   std::vector<std::string> paths,
                                   FSW_EVENT_CALLBACK *callback,
                                   void *context = nullptr);

    /**
     * @brief Creates a monitor whose type is the specified by @p name.
     *
     * The other parameters are forwarded to the fsw::monitor() constructor.
     *
     * @param name The monitor type.
     * @param paths The paths to watch.
     * @param callback The callback to invoke during the notification of a
     * change event.
     * @return The newly created monitor.
     * @throw libfsw_exception if a monitor of the type specified by @p name
     * cannot be found.
     * @see fsw::monitor()
     */
    static monitor *create_monitor(const std::string& name,
                                   std::vector<std::string> paths,
                                   FSW_EVENT_CALLBACK *callback,
                                   void *context = nullptr);

    /**
     * @brief Get the available monitor types.
     *
     * @return A vector with the available monitor types.
     */
    static std::vector<std::string> get_types();

    /**
     * @brief Checks whether a monitor of the type specified by @p name exists.
     *
     * @return `true` if @p name specifies a valid monitor type, `false`
     * otherwise.
     *
     * @param name The name of the monitor type to look for.
     * @return `true` if the type @p name exists, `false` otherwise.
     */
    static bool exists_type(const std::string& name);

    monitor_factory() = delete;
    monitor_factory(const monitor_factory& orig) = delete;
    monitor_factory& operator=(const monitor_factory& that) = delete;
  private:
    static std::map<std::string, fsw_monitor_type>& creators_by_string();
  };
}

#endif  /* FSW__MONITOR_FACTORY_H */