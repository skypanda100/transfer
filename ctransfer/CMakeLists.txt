cmake_minimum_required(VERSION 2.6)
project(ctransfer)

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
    endif ()
else ()
    set (CMAKE_C_STANDARD 99)
endif ()


set(SOURCE_FILES main.c config.h config.c def.h log.h log.c util.h util.c client.h client.c notify.h notify.c)
add_executable(ctransfer ${SOURCE_FILES})
target_link_libraries(ctransfer pthread)

INSTALL(TARGETS ctransfer
        RUNTIME DESTINATION bin
        )