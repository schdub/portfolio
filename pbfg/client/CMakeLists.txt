cmake_minimum_required(VERSION 3.10)
project(client)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 17)
set(TARGET client)

list(APPEND SOURCES
        client.cxx)

add_executable(${TARGET} ${SOURCES})

target_compile_definitions(${TARGET} PRIVATE
        UNICODE
        _UNICODE)

include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/../server/include)

target_link_libraries(${TARGET}
        )
