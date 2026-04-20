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
target_include_directories(fswatch_test PRIVATE ../.. .)
target_include_directories(fswatch_test PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(fswatch_test PUBLIC libfswatch)

if (BUILD_TESTING)
    find_program(SH_EXECUTABLE sh)
    find_program(GIT_EXECUTABLE git)

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND SH_EXECUTABLE)
        add_test(NAME inotify_access_events
                COMMAND ${SH_EXECUTABLE}
                        ${PROJECT_SOURCE_DIR}/test/inotify_access_events.sh
                        $<TARGET_FILE:fswatch>)
        set_tests_properties(inotify_access_events PROPERTIES
                LABELS "integration;inotify"
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

        if (HAVE_FANOTIFY)
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
        endif ()
    endif ()

    if (APPLE)
        add_executable(fsevents_missing_inode_test fsevents_missing_inode_test.cpp)
        target_include_directories(fsevents_missing_inode_test PRIVATE ../.. .)
        target_include_directories(fsevents_missing_inode_test PRIVATE ${PROJECT_BINARY_DIR})
        find_library(CORESERVICES_LIBRARY CoreServices)
        target_link_libraries(fsevents_missing_inode_test PUBLIC libfswatch ${CORESERVICES_LIBRARY})
        add_test(NAME fsevents_missing_inode_test COMMAND fsevents_missing_inode_test)
        set_tests_properties(fsevents_missing_inode_test PROPERTIES SKIP_RETURN_CODE 77)
    endif ()
endif ()
