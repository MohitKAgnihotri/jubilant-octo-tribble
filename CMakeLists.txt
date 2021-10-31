cmake_minimum_required(VERSION 3.20)
project(jubilant_octo_tribble C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
link_libraries(pthread)

add_executable(server
        server.c
        server.h)

add_executable(client
        client.c
        client.h commands.c commands.h)
