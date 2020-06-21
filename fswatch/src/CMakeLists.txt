set(SOURCE_FILES
        fswatch.cpp
        fswatch.hpp
        gettext.h)

# We'll need one global config file for both cmake and automake :P
set(COMPILE_DEFINITIONS
        -DPACKAGE="fswatch"
        -DPACKAGE_NAME="fswatch"
        -DPACKAGE_VERSION="1.14.0"
        -DPACKAGE_STRING="fswatch 1.14.0"
        -DPACKAGE_BUGREPORT="enrico.m.crisostomo@gmail.com"
        -DPACKAGE_URL="https://github.com/emcrisostomo/fswatch")

if (EXISTS ../libfswatch_config.h)
    set(SOURCE_FILES
            ${SOURCE_FILES}
            ../../libfswatch_config.h)

    set(COMPILE_DEFINITIONS
            ${COMPILE_DEFINITIONS}
            -DHAVE_CONFIG_H)
endif ()

add_executable(fswatch ${SOURCE_FILES})
target_include_directories(fswatch PUBLIC ../.. .)
target_link_libraries(fswatch LINK_PUBLIC libfswatch)
target_compile_definitions(fswatch PRIVATE ${COMPILE_DEFINITIONS})
