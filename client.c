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

void cursor_hide() {
    CONSOLE_CURSOR_INFO curInfo = {.dwSize = 1, .bVisible = FALSE};
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

int init_ui() {
    system("cls");
    int ret;
    printf("choose a game mode(control:\"wasd\",quit in game:\"q\"):\n");
    printf("1.singleplayer\n");
    printf("2.multiplayer\n");
    printf("3.quit\n");
    scanf("%d", &ret);
    return ret;
}

void singleplayer() {
    system("cls");

    cursor_hide();
    init_map(map);
    init_apple(map, apple);

    snake player;
    init_snake(map, &player, SNAKE_1);
    int direction = DEFAULT_DIRECTION;
    char input;
    memset(map_old, AIR, sizeof(map_old));
    render_map();
    while (1) {
        input = DEFAULT_INPUT;
        if (_kbhit()) {
            input = (char) getch();
        }
        process_input(&player, input, &direction);
        if (direction == QUIT_DIRECTION) {
            //player quit
            break;
        }
        if (player.current_direction != INIT_DIRECTION) {
            move_snake(map, apple, &player, SNAKE_1, &direction);
        }
        if (direction == DEAD_DIRECTION) {
            //player dead
            cursor_go(0, ROW);
            printf("game over\n");
            system("pause");
            break;
        }
        render_map();
        Sleep(TIME_WAIT);
    }
}

void multiplayer() {
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
        printf("failed to connect to the server.");
        system("pause");
        return;
    }

    //connected
    system("cls");
    cursor_hide();
    char senddata;
    memset(map_old, AIR, sizeof(map_old));
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

int main() {
    srand((unsigned) time(NULL));

    while (1) {
        int input = init_ui();
        if (input == 1) {
            singleplayer();
        } else if (input == 2) {
            multiplayer();
        } else if (input == 3) {
            break;
        }
    }
    return 0;
}