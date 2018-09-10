add_definitions(-DHAVE_CONFIG_H)
set(FSWATCH_SRC_FILES
        fswatch.cpp
        fswatch.hpp
        gettext.h
        ${PROJECT_BINARY_DIR}/cmake_config.h)

add_executable(fswatch ${FSWATCH_SRC_FILES})

# check for gettext and libintl
if (USE_NLS)
    if (Intl_LIBRARIES)
        target_link_libraries(fswatch PRIVATE ${Intl_LIBRARIES})
    endif ()
endif ()

target_include_directories(fswatch PUBLIC ../.. .)
target_include_directories(fswatch PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(fswatch PUBLIC libfswatch)

install(TARGETS fswatch DESTINATION bin)
