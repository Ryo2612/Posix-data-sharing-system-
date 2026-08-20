cmake_minimum_required(VERSION 3.29)
project(Daemon C)

set(CMAKE_C_STANDARD 11)

add_executable(Daemon daemon.c)

add_library(library STATIC library.c
        library.c)

add_executable(client1 client1.c
        client1.c)
target_link_libraries(client1 library)

add_executable(client2 client2.c
        client2.c
        client2.c)
target_link_libraries(client2 library)

add_executable(clientInvalid clientInvalid.c
        clientInvalid.c)
target_link_libraries(clientInvalid library)