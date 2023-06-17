//
// Created by V on 2023/4/25.
//

#include "share.h"

#if defined (__WIN32__)
#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define Socket SOCKET

int close(Socket s) {
    return closesocket(s);
}

void clear() {
    system("cls");
}

void curs_set(int flag) {
    CONSOLE_CURSOR_INFO cursor_info = {
        .dwSize = 1,
        .bVisible = flag
    };
    
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

void move(int x, int y) {
    COORD pos = {
        .X = (short) y,
        .Y = (short) x
    };

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
#elif __linux__
#include <curses.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define Socket int
#define printf(...) printw(__VA_ARGS__); refresh();
#define scanf(...) scanw(__VA_ARGS__)
#endif

void wait(unsigned int ms) {
    #if defined (__WIN32__)
    Sleep(ms);
    #elif __linux__
    usleep(1000 * ms);
    #endif
}

#define INFO_MENU 0
#define INFO_SINGLE 1
#define INFO_MULTI 2
#define INFO_DEAD 3

Socket openClientSocket(char *host_name) {
    Socket client_socket;
    struct sockaddr_in hints = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(host_name),
        .sin_port = htons(DEFAULT_PORT)
    };

    //create client socket
    if ((client_socket = socket(hints.sin_family, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        return -1;
    }

    //connect to server
    if (connect(client_socket, (struct sockaddr *) &hints, sizeof(hints)) == -1) {
        close(client_socket);
        return -2;
    }
    return client_socket;
}

void renderMap(Map map, char cache[][COL]) {
    for (int i = 0; i < ROW; i += 1) {
        for (int j = 0; j < COL; j += 1) {
            if (map.data[i][j] != cache[i][j]) {
                move(i, j);
                printf("%c", map.data[i][j]);
                cache[i][j] = map.data[i][j];
            }
        }
    }
    for (int i = 0; i < 2; i += 1) {
        if (map.score[i]) {
            move(3 + i, COL + 16);
            printf("%d", map.score[i]);
        }
    }
}

void printInfo(int flag, char *s) {
    if (s) {    //get server ip address
        struct in_addr tmp;
        
        #if defined (__linux__)
        echo();
        #endif
        curs_set(1);

        clear();
        while (1) {
            printf("Enter the server ip address(Quit: 'q'): ");
            scanf("%s", s);
            if (s[0] == 'q') {
                break;
            }
            if (inet_pton(AF_INET, s, &tmp) != 1) {
                printf("Invalid ip address, try again.\n");
            } else {
                break;
            }
        }

        #if defined (__linux__)
        noecho();
        #endif
        curs_set(0);
    } else {
        switch (flag) {
            case INFO_MENU:
                clear();
                printf("Choose a game mode(Select: 'w', 's'; Confirm: 'space'):\n");
                printf("  Single player.\n");
                printf("  Multiplayer.\n");
                printf("  Quit.\n");
                break;
            case INFO_DEAD:
                #if defined (__linux__)
                nodelay(stdscr, 0);
                #endif

                move(ROW, 0);
                printf("Game over.\n");
                printf("Press any key to continue.");
                getch();
                break;
            default:
                #if defined (__linux__)
                nodelay(stdscr, 1);
                #endif

                clear();
                move(0, COL + 1);
                printf("Control: 'w', 's', 'a', 'd'");
                move(1, COL + 1);
                printf("Quit: 'q'");
                for (int i = 0; i < flag; i += 1) {
                    move(i + 3, COL + 1);
                    printf("Player%d score: 0", i + 1);
                }
                break;
        }
    }
}

void singlePlayer() {
    Map map;
    Snake player;
    char cache[ROW][COL];
    char input;

    initMap(&map);
    initFood(&map);
    initSnake(&map, &player, 0);
    memset(cache, AIR, sizeof(cache));
    printInfo(INFO_SINGLE, 0);

    //start game
    while (map.space && player.current != dead) {
        #if defined (__WIN32__)
        if (kbhit()) {
            input = (char) getch();
            processInput(&player, input);
        }
        #elif __linux__
        if ((input = (char) getch()) != -1) {
            processInput(&player, input);
        }
        #endif
        
        moveSnake(&map, &player);
        map.score[0] = player.len - 1;
        renderMap(map, cache);
        wait(200);
    }
    printInfo(INFO_DEAD, 0);
}

void multiPlayer() {
    #if defined (__WIN32__)
    WSADATA wsa_data;

    //init WinSock
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
    #endif

    Map map;
    Socket client_socket;
    char cache[ROW][COL];
    char host_name[16];
    char send_data;

    printInfo(-1, host_name);

    if (host_name[0] != 'q') {
        printf("Connecting . . .\n");
        //create connect socket
        client_socket = openClientSocket(host_name);
        switch (client_socket) {
            case -1:
                printf("Failed to create client socket.\n");
                printf("Press any key to continue.");
                getch();
                break;
            case -2:
                printf("Failed to connect to the server.\n");
                printf("Press any key to continue.");
                getch();
                break;
            default:    //connected
                memset(cache, AIR, sizeof(cache));
                printInfo(INFO_MULTI, 0);
                while (1) {
                    //receive data from server
                    recv(client_socket, (char *) &map, sizeof(map), 0);
                    renderMap(map, cache);
                    if (!map.space) {
                        close(client_socket);
                        break;
                    }

                    #if defined (__WIN32__)
                    if (kbhit()) {
                        send_data = (char) getch();

                        //send data to server
                        send(client_socket, &send_data, 1, 0);
                    }
                    #elif __linux__
                    if ((send_data = (char) getch()) != -1) {
                        //send data to server
                        send(client_socket, &send_data, 1, 0);
                    }
                    #endif
                }
                printInfo(INFO_DEAD, 0);
                break;
        }
    }
    #if defined (__WIN32__)
    WSACleanup();
    #endif
}

int renderMenu() {
    int choice = 1;
    char input;
    
    printInfo(INFO_MENU, 0);
    while (1) {
        move(choice, 0);
        printf("->");
        move(choice, 0);
        input = (char) getch();
        switch (input) {
            case 'w':
                printf("  ");
                choice -= 1;
                if (choice == 0) {
                    choice = 3;
                }
                break;
            case 's':
                printf("  ");
                choice += 1;
                if (choice == 4) {
                    choice = 1;
                }
                break;
            case ' ':
                if (choice == 1) {
                    singlePlayer();
                    return 1;
                }
                if (choice == 2) {
                    multiPlayer();
                    return 1;
                }
                return 0;
            default:
                break;
        }
    }
}

int main() {
    #if defined (__linux__)
    initscr();
    cbreak();
    noecho();
    #endif
    curs_set(0);
    while (renderMenu()) {
    }
    curs_set(1);
    #if defined (__linux__)
    endwin();
    #endif
    return 0;
}