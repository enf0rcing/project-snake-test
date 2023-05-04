//
// Created by V on 2023/4/25.
//
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include "share.h"

short apple[2];
char map[ROW * COL], map_old[ROW * COL];

void cursor_show(int flag) {
    CONSOLE_CURSOR_INFO curInfo = {.dwSize = 1, .bVisible = flag};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void cursor_go(short x, short y) {
    COORD pos = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void render_map() {
    for (short i = 0; i < ROW; i++) {
        for (short j = 0; j < COL; j++) {
            if (map[i * COL + j] != map_old[i * COL + j]) {
                cursor_go(j, i);
                printf("%c", map[i * COL + j]);
            }
        }
    }
    memcpy(map_old, map, sizeof(map));
}

void print_info(int flag) {
    cursor_go(COL + 1, 0);
    printf("control: \"wasd\"");
    cursor_go(COL + 1, 1);
    printf("quit: \"q\"");
    if (flag) {
        cursor_go(COL + 1, 3);
        printf("player1 score: ");
        cursor_go(COL + 1, 4);
        printf("player2 score: ");
    } else {
        cursor_go(COL + 1, 3);
        printf("player score: ");
    }
}

void singleplayer() {
    system("cls");
    srand((unsigned) time(NULL));

    init_map(map);
    init_apple(map, apple);

    snake player;
    init_snake(map, &player, symbol[0]);
    int direction = DEFAULT_DIRECTION;
    char input;
    memset(map_old, AIR, sizeof(map_old));
    render_map();
    print_info(0);
    while (1) {
        input = DEFAULT_INPUT;
        if (_kbhit()) {
            input = (char) getch();
        }
        process_input(&player, input, &direction);
        if (direction == QUIT_DIRECTION) {
            //player quit
            cursor_go(0, ROW);
            system("pause");
            break;
        }
        if (player.current_direction != INIT_DIRECTION) {
            move_snake(map, apple, &player, symbol[0], &direction);
        }
        if (direction == DEAD_DIRECTION) {
            //player dead
            cursor_go(0, ROW);
            printf("game over\n");
            system("pause");
            break;
        }
        render_map();
        cursor_go(COL + 15, 3);
        printf("%d", player.len - 1);
        Sleep(TIME_WAIT);
    }
}

void multiplayer() {
    system("cls");
    cursor_show(1);

    //init winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //creat a socket to connect
    SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    char targetip[20];
    while (1) {
        printf("enter the server ip address: ");
        scanf("%s", targetip);
        if (inet_addr(targetip) == INADDR_NONE) {
            printf("invalid ip address, try again.\n");
        } else {
            break;
        }
    }
    struct sockaddr_in hints;
    memset(&hints, 0, sizeof(hints));
    hints.sin_family = AF_INET;
    hints.sin_addr.s_addr = inet_addr(targetip);
    hints.sin_port = htons(DEFAULT_PORT);

    //connect to the server
    int iResult = connect(ConnectSocket, (SOCKADDR *) &hints, sizeof(hints));
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        printf("failed to connect to the server.\n");
        system("pause");
        return;
    }

    //connected
    system("cls");
    cursor_show(0);

    char senddata;
    memset(map_old, AIR, sizeof(map_old));
    print_info(1);
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
                if (map[i * COL + j] == symbol[0]) {
                    score[0]++;
                } else if (map[i * COL + j] == symbol[1]) {
                    score[1]++;
                }
            }
        }
        cursor_go(COL + 16, 3);
        printf("%d ", score[0] - 1);
        cursor_go(COL + 16, 4);
        printf("%d ", score[1] - 1);

        //keep sending data to server
        senddata = DEFAULT_INPUT;
        if (_kbhit()) {
            senddata = (char) getch();
        }
        send(ConnectSocket, &senddata, 1, 0);
    }

    //clean up
    closesocket(ConnectSocket);
    WSACleanup();
}

int init_ui() {
    system("cls");

    cursor_show(0);
    printf("choose a game mode:\n");
    printf("   singleplayer\n");
    printf("   multiplayer\n");
    printf("   quit\n");
    short loc = 1;
    while (1) {
        cursor_go(0, loc);
        printf("->");
        char input = (char) getch();
        if (input == 'w') {
            cursor_go(0, loc);
            printf("  ");
            if (loc == 1) {
                loc = 3;
            } else {
                loc--;
            }
        } else if (input == 's') {
            cursor_go(0, loc);
            printf("  ");
            if (loc == 3) {
                loc = 1;
            } else {
                loc++;
            }
        } else if (input == 13) {
            switch (loc) {
                case 1:
                    singleplayer();
                    return 1;
                case 2:
                    multiplayer();
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