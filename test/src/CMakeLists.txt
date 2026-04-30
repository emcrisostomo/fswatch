#
# Copyright (c) 2014-2026 Enrico M. Crisostomo
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
set(SOURCE_FILES
        fswatch_test.cpp
        ${PROJECT_BINARY_DIR}/libfswatch/libfswatch_config.h)

add_executable(fswatch_test ${SOURCE_FILES})
target_include_directories(fswatch_test BEFORE PRIVATE ${PROJECT_BINARY_DIR})
target_include_directories(fswatch_test PRIVATE ../.. .)
target_link_libraries(fswatch_test PUBLIC libfswatch)

if (BUILD_TESTING)
    find_program(SH_EXECUTABLE sh)
    find_program(GIT_EXECUTABLE git)

    if (HAVE_INOTIFY_MONITOR)
        add_executable(inotify_stop_latency_test inotify_stop_latency_test.cpp)
        target_include_directories(inotify_stop_latency_test PRIVATE ../.. .)
        target_include_directories(inotify_stop_latency_test BEFORE PRIVATE ${PROJECT_BINARY_DIR})
        target_link_libraries(inotify_stop_latency_test PUBLIC libfswatch)
        add_test(NAME inotify_stop_latency_test COMMAND inotify_stop_latency_test)
        set_tests_properties(inotify_stop_latency_test PROPERTIES
                LABELS "integration;inotify"
                TIMEOUT 5)

        add_executable(inotify_stop_with_pending_events_test inotify_stop_with_pending_events_test.cpp)
        target_include_directories(inotify_stop_with_pending_events_test PRIVATE ../.. .)
        target_include_directories(inotify_stop_with_pending_events_test BEFORE PRIVATE ${PROJECT_BINARY_DIR})
        target_link_libraries(inotify_stop_with_pending_events_test PUBLIC libfswatch)
        add_test(NAME inotify_stop_with_pending_events_test COMMAND inotify_stop_with_pending_events_test)
        set_tests_properties(inotify_stop_with_pending_events_test PROPERTIES
                LABELS "integration;inotify"
                TIMEOUT 5)

        add_executable(inotify_stop_with_ready_events_test inotify_stop_with_ready_events_test.cpp)
        target_include_directories(inotify_stop_with_ready_events_test PRIVATE ../.. .)
        target_include_directories(inotify_stop_with_ready_events_test BEFORE PRIVATE ${PROJECT_BINARY_DIR})
        target_link_libraries(inotify_stop_with_ready_events_test PUBLIC libfswatch)
        add_test(NAME inotify_stop_with_ready_events_test COMMAND inotify_stop_with_ready_events_test)
        set_tests_properties(inotify_stop_with_ready_events_test PROPERTIES
                LABELS "integration;inotify"
                TIMEOUT 5)

        if (SH_EXECUTABLE)
            add_test(NAME inotify_basic_events
                    COMMAND ${SH_EXECUTABLE}
                            ${PROJECT_SOURCE_DIR}/test/inotify_basic_events.sh
                            $<TARGET_FILE:fswatch>)
            set_tests_properties(inotify_basic_events PROPERTIES
                    LABELS "integration;inotify"
                    TIMEOUT 15)

            add_test(NAME inotify_access_events
                    COMMAND ${SH_EXECUTABLE}
                            ${PROJECT_SOURCE_DIR}/test/inotify_access_events.sh
                            $<TARGET_FILE:fswatch>)
            set_tests_properties(inotify_access_events PROPERTIES
                    LABELS "integration;inotify"
                    TIMEOUT 15)

            add_test(NAME inotify_missing_root_rescan
                    COMMAND ${SH_EXECUTABLE}
                            ${PROJECT_SOURCE_DIR}/test/inotify_missing_root_rescan.sh
                            $<TARGET_FILE:fswatch>)
            set_tests_properties(inotify_missing_root_rescan PROPERTIES
                    LABELS "integration;inotify"
                    TIMEOUT 20)

            add_test(NAME inotify_watched_file_recreate
                    COMMAND ${SH_EXECUTABLE}
                            ${PROJECT_SOURCE_DIR}/test/inotify_watched_file_recreate.sh
                            $<TARGET_FILE:fswatch>)
            set_tests_properties(inotify_watched_file_recreate PROPERTIES
                    LABELS "integration;inotify"
                    TIMEOUT 20)

            add_test(NAME inotify_watched_directory_replace
                    COMMAND ${SH_EXECUTABLE}
                            ${PROJECT_SOURCE_DIR}/test/inotify_watched_directory_replace.sh
                            $<TARGET_FILE:fswatch>)
            set_tests_properties(inotify_watched_directory_replace PROPERTIES
                    LABELS "integration;inotify"
                    TIMEOUT 25)

            add_test(NAME inotify_burst_create
                    COMMAND ${SH_EXECUTABLE}
                            ${PROJECT_SOURCE_DIR}/test/inotify_burst_create.sh
                            $<TARGET_FILE:fswatch>)
            set_tests_properties(inotify_burst_create PROPERTIES
                    LABELS "integration;inotify"
                    TIMEOUT 20)

            add_test(NAME inotify_queue_drain
                    COMMAND ${SH_EXECUTABLE}
                            ${PROJECT_SOURCE_DIR}/test/inotify_queue_drain.sh
                            $<TARGET_FILE:fswatch>)
            set_tests_properties(inotify_queue_drain PROPERTIES
                    LABELS "integration;inotify"
                    TIMEOUT 25)

            add_test(NAME inotify_filter_root_path
                    COMMAND ${SH_EXECUTABLE}
                            ${PROJECT_SOURCE_DIR}/test/filter_root_path.sh
                            $<TARGET_FILE:fswatch>
                            inotify_monitor)
            set_tests_properties(inotify_filter_root_path PROPERTIES
                    LABELS "integration;inotify;filtering"
                    TIMEOUT 15)

            if (GIT_EXECUTABLE)
                add_test(NAME inotify_recursive_create
                        COMMAND ${SH_EXECUTABLE}
                                ${PROJECT_SOURCE_DIR}/test/inotify_recursive_create.sh
                                $<TARGET_FILE:fswatch>
                                ${GIT_EXECUTABLE})
                set_tests_properties(inotify_recursive_create PROPERTIES
                        LABELS "integration;inotify"
                        TIMEOUT 15)
            endif ()
        endif ()

    endif ()

    if (SH_EXECUTABLE)
        add_test(NAME poll_filter_root_path
                COMMAND ${SH_EXECUTABLE}
                        ${PROJECT_SOURCE_DIR}/test/filter_root_path.sh
                        $<TARGET_FILE:fswatch>
                        poll_monitor)
        set_tests_properties(poll_filter_root_path PROPERTIES
                LABELS "integration;poll;filtering"
                TIMEOUT 20)
    endif ()

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND SH_EXECUTABLE AND HAVE_FANOTIFY)
        add_test(NAME fanotify_basic
                COMMAND ${SH_EXECUTABLE}
                        ${PROJECT_SOURCE_DIR}/test/fanotify_basic.sh
                        $<TARGET_FILE:fswatch>)
        set_tests_properties(fanotify_basic PROPERTIES
                LABELS "integration;fanotify"
                SKIP_RETURN_CODE 77
                TIMEOUT 20)

        add_test(NAME fanotify_access_events
                COMMAND ${SH_EXECUTABLE}
                        ${PROJECT_SOURCE_DIR}/test/fanotify_access_events.sh
                        $<TARGET_FILE:fswatch>)
        set_tests_properties(fanotify_access_events PROPERTIES
                LABELS "integration;fanotify"
                SKIP_RETURN_CODE 77
                TIMEOUT 20)

        add_test(NAME fanotify_unlimited_properties
                COMMAND ${SH_EXECUTABLE}
                        ${PROJECT_SOURCE_DIR}/test/fanotify_unlimited_properties.sh
                        $<TARGET_FILE:fswatch>)
        set_tests_properties(fanotify_unlimited_properties PROPERTIES
                LABELS "integration;fanotify"
                SKIP_RETURN_CODE 77
                TIMEOUT 20)

        add_test(NAME fanotify_filter_root_path
                COMMAND ${SH_EXECUTABLE}
                        ${PROJECT_SOURCE_DIR}/test/filter_root_path.sh
                        $<TARGET_FILE:fswatch>
                        fanotify_monitor)
        set_tests_properties(fanotify_filter_root_path PROPERTIES
                LABELS "integration;fanotify;filtering"
                SKIP_RETURN_CODE 77
                TIMEOUT 20)
    endif ()

    if (APPLE)
        add_executable(fsevents_missing_inode_test fsevents_missing_inode_test.cpp)
        target_include_directories(fsevents_missing_inode_test BEFORE PRIVATE ${PROJECT_BINARY_DIR})
        target_include_directories(fsevents_missing_inode_test PRIVATE ../.. .)
        find_library(CORESERVICES_LIBRARY CoreServices)
        target_link_libraries(fsevents_missing_inode_test PUBLIC libfswatch ${CORESERVICES_LIBRARY})
        add_test(NAME fsevents_missing_inode_test COMMAND fsevents_missing_inode_test)
        set_tests_properties(fsevents_missing_inode_test PROPERTIES SKIP_RETURN_CODE 77)

        add_executable(fsevents_cf_paths_test fsevents_cf_paths_test.cpp)
        target_include_directories(fsevents_cf_paths_test BEFORE PRIVATE ${PROJECT_BINARY_DIR})
        target_include_directories(fsevents_cf_paths_test PRIVATE ../.. .)
        target_link_libraries(fsevents_cf_paths_test PUBLIC libfswatch ${CORESERVICES_LIBRARY})
        add_test(NAME fsevents_cf_paths_test COMMAND fsevents_cf_paths_test)

        if (SH_EXECUTABLE)
            add_test(NAME fsevents_filter_root_path
                    COMMAND ${SH_EXECUTABLE}
                            ${PROJECT_SOURCE_DIR}/test/filter_root_path.sh
                            $<TARGET_FILE:fswatch>
                            fsevents_monitor)
            set_tests_properties(fsevents_filter_root_path PROPERTIES
                    LABELS "integration;fsevents;filtering"
                    SKIP_RETURN_CODE 77
                    TIMEOUT 20)
        endif ()
    endif ()
endif ()
