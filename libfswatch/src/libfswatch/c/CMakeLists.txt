set(LIBRARY_NAME "c${PACKAGE}")

set(Header_Files
        "cevent.h"
        "cfilter.h"
        "cmonitor.h"
        "fsw_error.h"
        "libfswatch.h"
        "libfswatch_log.h"
        "libfswatch_types.h"
        )
source_group("${LIBRARY_NAME} Header Files" FILES "${Header_Files}")

set(Source_Files "cevent.cpp"
        "libfswatch.cpp"
        "libfswatch_log.cpp")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND Source_Files "windows/realpath.c")
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")

source_group("${LIBRARY_NAME} Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" STATIC "${Header_Files}" "${Source_Files}")
if (MSVC)
    set_target_properties("${LIBRARY_NAME}" PROPERTIES COMPILE_FLAGS "/EHsc")
endif (MSVC)
target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:include>"
)

target_link_libraries("${LIBRARY_NAME}" PRIVATE "cxx${PACKAGE}" "gettext")

set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
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
