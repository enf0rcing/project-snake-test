cmake_minimum_required(VERSION 3.25)
project(snake-test C)

set(CMAKE_C_STANDARD 11)

add_executable(client client.c share.c)
add_executable(server server.c share.c)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(client ws2_32)
    target_link_libraries(server ws2_32 pthread)
elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(client ncurses)
endif()