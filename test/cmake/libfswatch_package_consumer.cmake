#
# Copyright (c) 2026 Enrico M. Crisostomo
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.
#

foreach(required_var TEST_BUILD_DIR TEST_WORK_DIR TEST_GENERATOR)
    if (NOT DEFINED ${required_var})
        message(FATAL_ERROR "${required_var} is required")
    endif ()
endforeach()

get_filename_component(TEST_BUILD_DIR "${TEST_BUILD_DIR}" ABSOLUTE)
get_filename_component(TEST_WORK_DIR "${TEST_WORK_DIR}" ABSOLUTE)
string(FIND "${TEST_WORK_DIR}/" "${TEST_BUILD_DIR}/" work_dir_in_build_dir)
if (NOT work_dir_in_build_dir EQUAL 0)
    message(FATAL_ERROR "Refusing to write outside the build tree: ${TEST_WORK_DIR}")
endif ()

set(install_dir "${TEST_WORK_DIR}/install")
set(consumer_source_dir "${TEST_WORK_DIR}/consumer")
set(consumer_build_dir "${TEST_WORK_DIR}/consumer-build")

file(REMOVE_RECURSE "${TEST_WORK_DIR}")
file(MAKE_DIRECTORY "${consumer_source_dir}" "${consumer_build_dir}")

set(config_args)
if (DEFINED TEST_CONFIG AND NOT TEST_CONFIG STREQUAL "")
    list(APPEND config_args --config "${TEST_CONFIG}")
endif ()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${TEST_BUILD_DIR}" --prefix "${install_dir}" ${config_args}
    RESULT_VARIABLE install_result)
if (NOT install_result EQUAL 0)
    message(FATAL_ERROR "Could not install libfswatch package test prefix")
endif ()

file(WRITE "${consumer_source_dir}/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.14)
project(libfswatch_package_consumer LANGUAGES CXX)

find_package(libfswatch CONFIG REQUIRED
  PATHS ${CMAKE_PREFIX_PATH}
  NO_DEFAULT_PATH)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE libfswatch::libfswatch)
]=])

file(WRITE "${consumer_source_dir}/main.cpp" [=[
#include <libfswatch/c/libfswatch.h>

int main()
{
  return fsw_init_library() == FSW_OK ? 0 : 1;
}
]=])

set(generator_args -G "${TEST_GENERATOR}")
if (DEFINED TEST_GENERATOR_PLATFORM AND NOT TEST_GENERATOR_PLATFORM STREQUAL "")
    list(APPEND generator_args -A "${TEST_GENERATOR_PLATFORM}")
endif ()
if (DEFINED TEST_GENERATOR_TOOLSET AND NOT TEST_GENERATOR_TOOLSET STREQUAL "")
    list(APPEND generator_args -T "${TEST_GENERATOR_TOOLSET}")
endif ()

set(compiler_args)
if (DEFINED TEST_CXX_COMPILER AND NOT TEST_CXX_COMPILER STREQUAL "")
    list(APPEND compiler_args "-DCMAKE_CXX_COMPILER=${TEST_CXX_COMPILER}")
endif ()

execute_process(
    COMMAND "${CMAKE_COMMAND}"
        -S "${consumer_source_dir}"
        -B "${consumer_build_dir}"
        ${generator_args}
        ${compiler_args}
        "-DCMAKE_PREFIX_PATH=${install_dir}"
    RESULT_VARIABLE configure_result)
if (NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Could not configure libfswatch package consumer")
endif ()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${consumer_build_dir}" ${config_args}
    RESULT_VARIABLE build_result)
if (NOT build_result EQUAL 0)
    message(FATAL_ERROR "Could not build libfswatch package consumer")
endif ()
