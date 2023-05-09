//
// Created by V on 2023/4/25.
//
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <winsock2.h>
#include "share.h"

char map[ROW * COL], mapOld[ROW * COL];

void cursor_show(int flag) {
    CONSOLE_CURSOR_INFO cursorInfo = {.dwSize = 1, .bVisible = flag};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void cursor_go(short x, short y) {
    COORD pos = {.X = x, .Y = y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void render_map() {
    for (short i = 0; i < ROW; i++) {
        for (short j = 0; j < COL; j++) {
            if (map[i * COL + j] != mapOld[i * COL + j]) {
                cursor_go(j, i);
                printf("%c", map[i * COL + j]);
            }
        }
    }
    memcpy(mapOld, map, sizeof(map));
}

void print_info(int flag) {
    cursor_go(COL + 1, 0);
    printf("control: \"wasd\"");
    cursor_go(COL + 1, 1);
    printf("quit: \"q\"");
    for (int i = 0; i < flag; i++) {
        cursor_go(COL + 1, i + 3);
        printf("player%d score: ", i + 1);
    }
}

void single_player() {
    system("cls");
    srand(time(NULL));
    int apple[2];

    init_map(map);
    init_apple(map, apple);

    snake player;
    init_snake(map, playerSymbol[0], &player);
    char input;
    memset(mapOld, AIR, sizeof(mapOld));
    render_map();
    print_info(1);
    while (1) {
        input = DEFAULT_INPUT;
        if (_kbhit()) {
            input = (char) getch();
        }
        process_input(input, &player);
        if (player.directionNew == QUIT_DIRECTION) {
            //player quit
            cursor_go(0, ROW);
            system("pause");
            break;
        }
        if (player.direction != INIT_DIRECTION) {
            move_snake(map, apple, &player);
        }
        if (player.directionNew == DEAD_DIRECTION) {
            //player dead
            cursor_go(0, ROW);
            printf("game over\n");
            system("pause");
            break;
        }
        render_map();
        cursor_go(COL + 16, 3);
        printf("%d", player.len - 1);
        Sleep(TIME_WAIT);
    }
}

void multi_player() {
    system("cls");
    cursor_show(1);

    //init WinSock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //creat a socket to connect
    SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    char targetIp[20];
    while (1) {
        printf("enter the server ip address: ");
        scanf("%s", targetIp);
        if (inet_addr(targetIp) == INADDR_NONE) {
            printf("invalid ip address, try again.\n");
        } else {
            break;
        }
    }
    struct sockaddr_in hints;
    memset(&hints, 0, sizeof(hints));
    hints.sin_family = AF_INET;
    hints.sin_addr.s_addr = inet_addr(targetIp);
    hints.sin_port = htons(DEFAULT_PORT);

    //connect to the server
    if (connect(ConnectSocket, (SOCKADDR *) &hints, sizeof(hints)) == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
        printf("failed to connect to the server.\n");
        system("pause");
        return;
    }

    //connected
    system("cls");
    cursor_show(0);

    char sendData;
    memset(mapOld, AIR, sizeof(mapOld));
    print_info(2);
    while (1) {
        //receive data from server
        recv(ConnectSocket, map, ROW * COL, 0);

        if (map[0] == RESTART) {
            cursor_go(0, ROW);
            printf("game over\n");
            system("pause");
            break;
        }
        render_map();
        int score[2] = {0};
        for (int i = 0; i < ROW; i++) {
            for (int j = 0; j < COL; j++) {
                if (map[i * COL + j] == playerSymbol[0]) {
                    score[0]++;
                } else if (map[i * COL + j] == playerSymbol[1]) {
                    score[1]++;
                }
            }
        }
        cursor_go(COL + 16, 3);
        printf("%d ", score[0] - 1);
        cursor_go(COL + 16, 4);
        printf("%d ", score[1] - 1);

        //keep sending data to server
        sendData = DEFAULT_INPUT;
        if (_kbhit()) {
            sendData = (char) getch();
        }
        send(ConnectSocket, &sendData, 1, 0);
    }
    //clean up
    closesocket(ConnectSocket);
    WSACleanup();
}

int init_ui() {
    system("cls");
    cursor_show(0);

    printf("choose a game mode:\n");
    printf("   single_player\n");
    printf("   multi_player\n");
    printf("   quit\n");
    short loc = 1;
    while (1) {
        cursor_go(0, loc);
        printf("->");
        char input = (char) getch();
        if (input == 'w') {
            cursor_go(0, loc);
            printf("  ");
            loc--;
            if (loc == 0) {
                loc = 3;
            }
        } else if (input == 's') {
            cursor_go(0, loc);
            printf("  ");
            loc++;
            if (loc == 4) {
                loc = 1;
            }
        } else if (input == 13) {
            switch (loc) {
                case 1:
                    single_player();
                    return 1;
                case 2:
                    multi_player();
                    return 1;
                case 3:
                    return 0;
                default:
                    break;
            }
        }
    }
}

int main() {
    while (init_ui()) {
    }
    return 0;
}