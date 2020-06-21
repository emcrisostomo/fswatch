include_directories(../.. .)

set(SOURCE_FILES
        fswatch_test.c)

set(COMPILE_DEFINITIONS "")

if (EXISTS ../libfswatch_config.h)
    set(SOURCE_FILES
            ${SOURCE_FILES}
            ../../libfswatch_config.h)

    set(COMPILE_DEFINITIONS
            ${COMPILE_DEFINITIONS}
            -DHAVE_CONFIG_H)
endif ()

add_executable(fswatch_test ${SOURCE_FILES})
target_link_libraries(fswatch_test LINK_PUBLIC libfswatch)
target_compile_definitions(fswatch_test PRIVATE ${COMPILE_DEFINITIONS})
