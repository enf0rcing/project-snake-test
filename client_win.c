//
// Created by V on 2023/4/25.
//

#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "share.h"

#define INFO_UI 0
#define INFO_SINGLE 1
#define INFO_MULTI 2
#define INFO_DEAD 3

void cursorShow(int flag) {
    CONSOLE_CURSOR_INFO cursor_info = {
        .dwSize = 1,
        .bVisible = flag
    };
    
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

void cursorGo(int x, int y) {
    COORD pos = {
        .X = (short) y,
        .Y = (short) x
    };

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void renderMap(Map map, char cache[][COL]) {
    int score[2] = {0};

    for (int i = 0; i < ROW; i += 1) {
        for (int j = 0; j < COL; j += 1) {
            if (map.data[i][j] != cache[i][j]) {
                cursorGo(i, j);
                printf("%c", map.data[i][j]);
                cache[i][j] = map.data[i][j];
            }
            for (int k = 0; k < 2; k += 1) {
                if (map.data[i][j] == Snake_Symbol[k]) {
                    score[k] += 1;
                }
            }
        }
    }
    for (int i = 0; i < 2; i += 1) {
        if (score[i]) {
            cursorGo(3 + i, COL + 16);
            printf("%d", score[i] - 1);
        }
    }
}

void printInfo(int flag) {
    switch (flag) {
        case INFO_DEAD:
            cursorShow(1);

            cursorGo(ROW, 0);
            printf("Game over.\n");
            system("pause");
            break;
        case INFO_UI:
            cursorShow(0);

            system("cls");
            printf("Choose a game mode:\n");
            printf("   Single player.\n");
            printf("   Multiplayer.\n");
            printf("   Quit.\n");
            break;
        default:
            cursorShow(0);
            
            system("cls");
            cursorGo(0, COL + 1);
            printf("Control: \"wasd\"");
            cursorGo(1, COL + 1);
            printf("Quit: \"q\"");
            for (int i = 0; i < flag; i += 1) {
                cursorGo(i + 3, COL + 1);
                printf("Player%d score: ", i + 1);
            }
            break;
    }
}

SOCKET openClientSock(char *host_name) {
    SOCKET client_socket;
    struct sockaddr_in hints = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(host_name),
        .sin_port = htons(DEFAULT_PORT)
    };

    //create client socket
    if ((client_socket = socket(hints.sin_family, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        WSACleanup();
        return -1;
    }

    //connect to server
    if (connect(client_socket, (struct sockaddr *) &hints, sizeof(hints)) == SOCKET_ERROR) {
        closesocket(client_socket);
        WSACleanup();
        return -2;
    }
    return client_socket;
}

void singlePlayer() {
    Map map;
    Snake player;
    char cache[ROW][COL];
    char input;

    initMap(&map);
    initFood(&map);
    initSnake(&map, &player, Snake_Symbol[0]);
    memset(cache, AIR, sizeof(cache));
    printInfo(INFO_SINGLE);

    //start game
    while (map.space && player.current != dead) {
        if (kbhit()) {
            input = (char) getch();
            processInput(&player, input);
        }
        moveSnake(&map, &player);
        renderMap(map, cache);
        Sleep(200);
    }
    printInfo(INFO_DEAD);
}

void multiPlayer() {
    Map map;
    WSADATA wsa_data;
    SOCKET client_socket;
    struct in_addr tmp;
    char cache[ROW][COL];
    char host_name[16] = {0};
    char send_data;

    //init WinSock
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    cursorShow(1);
    system("cls");
    while (host_name[0] != 'q') {
        //get server ip address
        printf("Enter the server ip address(Quit: 'q'): ");
        scanf("%s", host_name);
        if (inet_pton(AF_INET, host_name, &tmp) != 1) {
            printf("Invalid ip address, try again.\n");
        } else {
            printf("Connecting . . .\n");
            //create connect socket
            client_socket = openClientSock(host_name);
            switch (client_socket) {
                case -1:
                    printf("Failed to create client socket.\n");
                    system("pause");
                    return;
                case -2:
                    printf("Failed to connect to the server.\n");
                    system("pause");
                    return;
                default:    //connected
                    memset(cache, AIR, sizeof(cache));
                    printInfo(INFO_MULTI);
                    while (1) {
                        //receive data from server
                        recv(client_socket, (char *) &map, sizeof(map), 0);
                        renderMap(map, cache);
                        if (!map.space) {
                            closesocket(client_socket);
                            break;
                        }

                        if (kbhit()) {
                            send_data = (char) getch();
                            send(client_socket, &send_data, 1, 0);
                        }
                    }
                    printInfo(INFO_DEAD);
                    return;
            }
        }
    }
}

int renderMenu() {
    int choice = 1;
    char input;
    
    printInfo(INFO_UI);
    while (1) {
        cursorGo(choice, 0);
        printf("->");
        cursorGo(choice, 0);
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
            case 13:
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
    while (renderMenu()) {
    }
    return 0;
}