//
// Created by V on 2023/4/25.
//

#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <winsock2.h>
#include "share.h"

Map map;
char cache[ROW][COL];

void cursorShow(int flag) {
    CONSOLE_CURSOR_INFO cursor_info = {
        .dwSize = 1,
        .bVisible = flag
    };
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

void cursorGo(int x, int y) {
    COORD pos = {
        .X = (short) x,
        .Y = (short) y
    };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void renderMap() {
    int score[2] = {0};

    for (int i = 0; i < ROW; i += 1) {
        for (int j = 0; j < COL; j += 1) {
            if (map.data[i][j] != cache[i][j]) {
                cursorGo(j, i);
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
            cursorGo(COL + 16, 3 + i);
            printf("%d", score[i] - 1);
        }
    }
}

void printInfo(int flag) {
    if (flag == 0) {
        cursorGo(0, ROW);
        printf("Game over.\n");
    } else {
        cursorGo(COL + 1, 0);
        printf("Control: \"wasd\"");
        cursorGo(COL + 1, 1);
        printf("Quit: \"q\"");
        for (int i = 0; i < flag; i += 1) {
            cursorGo(COL + 1, i + 3);
            printf("Player%d score: ", i + 1);
        }
    }
}

void singlePlayer() {
    system("cls");
    srand(time(0));

    Snake player;
    char input;

    initMap(&map);
    initFood(&map);
    initSnake(&map, &player, Snake_Symbol[0]);

    memset(cache, AIR, sizeof(cache));
    renderMap();
    printInfo(1);

    //start game
    while (map.space && player.current != dead) {
        if (kbhit()) {
            input = (char) getch();
        }
        processInput(&player, input);
        moveSnake(&map, &player);
        renderMap();
        Sleep(200);
    }
    printInfo(0);
    system("pause");
}

void multiPlayer() {
    system("cls");
    cursorShow(1);

    WSADATA wsa_data;
    SOCKET connect_socket;
    struct sockaddr_in hints;
    char server_ip[16];
    char send_data;

    //init WinSock
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    //create connect socket
    connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //get server ip address
    while (1) {
        printf("Enter the server ip address: ");
        scanf("%s", server_ip);
        if (inet_addr(server_ip) == INADDR_NONE) {
            printf("Invalid ip address, try again.\n");
        } else {
            break;
        }
    }

    //connect to the server
    memset(&hints, 0, sizeof(hints));
    hints.sin_family = AF_INET;
    hints.sin_addr.s_addr = inet_addr(server_ip);
    hints.sin_port = htons(DEFAULT_PORT);
    if (connect(connect_socket, (SOCKADDR *) &hints, sizeof(hints)) == SOCKET_ERROR) {
        closesocket(connect_socket);
        WSACleanup();
        printf("Failed to connect to the server.\n");
        system("pause");
        return;
    }

    //connected
    system("cls");
    cursorShow(0);

    memset(cache, AIR, sizeof(cache));
    printInfo(2);
    while (1) {
        //receive data from server
        recv(connect_socket, (char *) &map, sizeof(map), 0);
        renderMap();

        if (!map.space) {
            shutdown(connect_socket, SD_BOTH);
            break;
        }
        //send data to server
        if (kbhit()) {
            send_data = (char) getch();
            send(connect_socket, &send_data, 1, 0);
        }
    }
    //clean up
    closesocket(connect_socket);
    WSACleanup();

    printInfo(0);
    system("pause");
}

int initUi() {
    system("cls");
    cursorShow(0);
    printf("Choose a game mode:\n");
    printf("   Single player.\n");
    printf("   Multiplayer.\n");
    printf("   Quit.\n");

    int choice = 1;
    char input;

    while (1) {
        cursorGo(0, choice);
        printf("->");
        cursorGo(0, choice);
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
    while (initUi()) {
    }
    return 0;
}