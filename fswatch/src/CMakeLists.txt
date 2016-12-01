include_directories(../.. .)

set(SOURCE_FILES
        fswatch.cpp
        fswatch.hpp
        gettext.h
        ../../libfswatch_config.h)

add_executable(fswatch ${SOURCE_FILES})
target_link_libraries(fswatch LINK_PUBLIC libfswatch)