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

void cursor_go(int x, int y) {
    COORD pos = {.X = (short) x, .Y = (short) y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void render_map(int *s1, int *s2) {
    for (int i = 0; i < ROW; i += 1) {
        for (int j = 0; j < COL; j += 1) {
            if (map[i * COL + j] != mapOld[i * COL + j]) {
                cursor_go(j, i);
                printf("%c", map[i * COL + j]);
                mapOld[i * COL + j] = map[i * COL + j];
            }
            if (s1 && map[i * COL + j] == playerSymbol[0]) {
                *s1 += 1;
            }
            if (s2 && map[i * COL + j] == playerSymbol[1]) {
                *s2 += 1;
            }
        }
    }
}

void print_info(int flag) {
    cursor_go(COL + 1, 0);
    printf("Control: \"wasd\"");
    cursor_go(COL + 1, 1);
    printf("Quit: \"q\"");
    for (int i = 0; i < flag; i += 1) {
        cursor_go(COL + 1, i + 3);
        printf("Player%d score: ", i + 1);
    }
}

void single_player() {
    system("cls");
    srand(time(0));
    int apple[2];

    init_map(map);
    init_apple(map, apple);

    Snake player;
    init_snake(map, playerSymbol[0], &player);
    char input;
    memset(mapOld, AIR, sizeof(mapOld));
    render_map(0, 0);
    print_info(1);
    while (1) {
        input = DEFAULT_INPUT;
        if (kbhit()) {
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
            printf("Game over\n");
            system("pause");
            break;
        }
        render_map(0, 0);
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

    //create a socket to connect
    SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    char targetIp[20];
    while (1) {
        printf("Enter the server ip address: ");
        scanf("%s", targetIp);
        if (inet_addr(targetIp) == INADDR_NONE) {
            printf("Invalid ip address, try again.\n");
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
        printf("Failed to connect to the server.\n");
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
            printf("Game over\n");
            system("pause");
            break;
        }
        int score[2] = {0};
        render_map(&score[0], &score[1]);
        cursor_go(COL + 16, 3);
        printf("%d ", score[0] - 1);
        cursor_go(COL + 16, 4);
        printf("%d ", score[1] - 1);

        //keep sending data to server
        sendData = DEFAULT_INPUT;
        if (kbhit()) {
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

    printf("Choose a game mode:\n");
    printf("   Single player\n");
    printf("   Multiplayer\n");
    printf("   Quit\n");
    int choose = 1;
    while (1) {
        cursor_go(0, choose);
        printf("->");
        char input = (char) getch();
        if (input == 'w') {
            cursor_go(0, choose);
            printf("  ");
            choose -= 1;
            if (choose == 0) {
                choose = 3;
            }
        } else if (input == 's') {
            cursor_go(0, choose);
            printf("  ");
            choose += 1;
            if (choose == 4) {
                choose = 1;
            }
        } else if (input == 13) {
            if (choose == 1) {
                single_player();
                return 1;
            }
            if (choose == 2) {
                multi_player();
                return 1;
            }
            return 0;
        }
    }
}

int main() {
    while (init_ui()) {
    }
    return 0;
}