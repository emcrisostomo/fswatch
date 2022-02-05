set(LIBRARY_NAME "${PACKAGE}_cxx_windows")

set(Header_Files "win_directory_change_event.hpp"
        "win_error_message.hpp"
        "win_handle.hpp"
        "win_paths.hpp"
        "win_strings.hpp"
        "../windows_monitor.hpp"
        )

set(Source_Files
        "win_directory_change_event.cpp"
        "win_error_message.cpp"
        "win_handle.cpp"
        "win_paths.cpp"
        "win_strings.cpp"
        "../windows_monitor.cpp"
        )

check_include_file_cxx("sys/cygwin.h" HAVE_CYGWIN)

if (MSVC)
    list(APPEND Source_Files "win_paths.cpp")
else ()
    list(APPEND Source_Files "cygwin_paths.cpp")
endif (MSVC)

set(HAVE_WINDOWS ON CACHE BOOL "Enable Windows support")

source_group("${LIBRARY_NAME} Header Files" FILES "${Header_Files}")
source_group("${LIBRARY_NAME} Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" STATIC "${Header_Files}" "${Source_Files}")
target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:include>"
)
target_link_libraries("${LIBRARY_NAME}" PRIVATE "c${PACKAGE}" "cxx${PACKAGE}" "gettext")
set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        CXX
)

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
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
install(EXPORT "${LIBRARY_NAME}Targets" DESTINATION "share/libfswatch")
