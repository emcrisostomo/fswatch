set(LIBRARY_NAME "cxx${PACKAGE}")

set(Header_Files "event.hpp"
        "filter.hpp"
        "libfswatch_exception.hpp"
        "libfswatch_map.hpp"
        "libfswatch_set.hpp"
        "monitor.hpp"
        "monitor_factory.hpp"
        "path_utils.hpp"
        "poll_monitor.hpp"
        )

set(Source_Files "event.cpp"
        "filter.cpp"
        "libfswatch_exception.cpp"
        "monitor.cpp"
        "monitor_factory.cpp"
        "path_utils.cpp"
        "poll_monitor.cpp"
        )

check_cxx_symbol_exists(thread_local cstdlib HAVE_CXX_THREAD_LOCAL)

check_include_file_cxx("sys/inotify.h" HAVE_SYS_INOTIFY_H)

if (HAVE_SYS_INOTIFY_H)
    list(APPEND Header_Files "inotify_monitor.hpp")
    list(APPEND Source_Files "inotify_monitor.cpp")
endif (HAVE_SYS_INOTIFY_H)

check_include_file_cxx("sys/event.h" HAVE_SYS_EVENT_H)

if (HAVE_SYS_EVENT_H)
    list(APPEND Header_Files "kqueue_monitor.hpp")
    list(APPEND Source_Files "kqueue_monitor.cpp")
endif (HAVE_SYS_EVENT_H)

check_include_file_cxx("port.h" HAVE_PORT_H)

if (HAVE_PORT_H)
    list(APPEND Header_Files "fen_monitor.hpp")
    list(APPEND Source_Files "fen_monitor.cpp")
endif (HAVE_PORT_H)

check_cxx_symbol_exists(FindFirstChangeNotification "windows.h" HAVE_WINDOWS_HEADER)

if (HAVE_WINDOWS_HEADER)
    include_directories("./windows")
endif (HAVE_WINDOWS_HEADER)

set(Extra_Libs "")

if (APPLE)
    check_include_file_cxx("CoreServices/CoreServices.h" HAVE_FSEVENTS_FILE_EVENTS)

    if (HAVE_FSEVENTS_FILE_EVENTS)
        find_library(CORESERVICES_LIBRARY CoreServices)
        set(Extra_Libs "${CORESERVICES_LIBRARY}")

        list(APPEND Header_Files "fsevents_monitor.hpp")
        list(APPEND Source_Files "fsevents_monitor.cpp")

    endif (HAVE_FSEVENTS_FILE_EVENTS)
endif (APPLE)

source_group("${LIBRARY_NAME} Header Files" FILES "${Header_Files}")
source_group("${LIBRARY_NAME} Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" STATIC "${Header_Files}" "${Source_Files}")
target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
        "$<INSTALL_INTERFACE:include>"
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory("./windows")
    target_link_libraries("${LIBRARY_NAME}" PUBLIC "${PACKAGE}_cxx_windows")
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_subdirectory("./string")
target_link_libraries("${LIBRARY_NAME}" PUBLIC "${PACKAGE}_string_utils" "${Extra_Libs}")
target_link_libraries("${LIBRARY_NAME}" PRIVATE "c${PACKAGE}" "gettext")

set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        CXX
)

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "1.0.0")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "1")

# install rules
set(installable_libs "${LIBRARY_NAME}")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        DESTINATION "lib"
        EXPORT "${LIBRARY_NAME}Targets")
install(FILES ${Header_Files} "${_export_file}" DESTINATION "include")
