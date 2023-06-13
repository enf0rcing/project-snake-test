cmake_minimum_required(VERSION 3.25)
project(snake-test C)

set(CMAKE_C_STANDARD 11)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    link_libraries(ws2_32)

    add_executable(client client_win.c share.c)
    add_executable(server server_win.c share.c)
    target_link_libraries(server pthread)
elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    add_executable(client client_linux.c share.c)
    target_link_libraries(client ncurses)
    add_executable(server server_linux.c share.c)
endif()