cmake_minimum_required(VERSION 3.25)
project(snake-test C)

set(CMAKE_C_STANDARD 11)

link_libraries(ws2_32)

add_executable(client client.c share.c)
add_executable(server server.c share.c)
target_link_libraries(server pthread)