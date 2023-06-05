//
// Created by V on 2023/4/25.
//
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <winsock2.h>
#include "share.h"

char map[ROW * COL + 1], cache[ROW * COL];

void cursor_show(int flag) {
    CONSOLE_CURSOR_INFO cursorInfo = {.dwSize = 1, .bVisible = flag};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void cursor_go(int x, int y) {
    COORD pos = {.X = (short) x, .Y = (short) y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void render_map() {
    int score[2] = {0};
    for (int i = 0; i < ROW; i += 1) {
        for (int j = 0; j < COL; j += 1) {
            int pos = Pos(i, j);
            if (map[pos] != cache[pos]) {
                cursor_go(j, i);
                printf("%c", map[pos]);
                cache[pos] = map[pos];
            }
            for (int k = 0; k < 2; k += 1) {
                if (map[pos] == SnakeSymbol[k]) {
                    score[k] += 1;
                }
            }
        }
    }
    for (int i = 0; i < 2; i += 1) {
        if (score[i]) {
            cursor_go(COL + 16, 3 + i);
            printf("%d", score[i] - 1);
        }
    }
}

void print_info(int flag) {
    if (flag == 0) {
        cursor_go(0, ROW);
        printf("Game over.\n");
        printf("Press any key to continue . . .\n");
    } else {
        cursor_go(COL + 1, 0);
        printf("Control: \"wasd\"");
        cursor_go(COL + 1, 1);
        printf("Quit: \"q\"");
        for (int i = 0; i < flag; i += 1) {
            cursor_go(COL + 1, i + 3);
            printf("Player%d score: ", i + 1);
        }
    }
}

void single_player() {
    system("cls");
    srand(time(0));

    init_map(map);
    int apple[2];
    init_apple(map, apple);

    Snake player;
    init_snake(map, SnakeSymbol[0], &player);

    memset(cache, AIR, sizeof(cache));
    render_map();
    print_info(1);

    //start game
    while (player.current != dead) {
        char input = 0;
        if (kbhit()) {
            input = (char) getch();
        }
        process_input(input, &player);
        move_snake(map, apple, &player);
        render_map();
        Sleep(200);
    }
    print_info(0);
    getch();
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

    memset(cache, AIR, sizeof(cache));
    print_info(2);
    while (1) {
        //receive data from server
        recv(ConnectSocket, map, sizeof(map), 0);
        if (!map[ROW * COL]) {
            break;
        }
        render_map();

        //send data to server
        if (kbhit()) {
            char sendData = (char) getch();
            send(ConnectSocket, &sendData, 1, 0);
        }
    }
    print_info(0);
    char sendData = (char) getch();
    send(ConnectSocket, &sendData, 1, 0);
    //clean up
    closesocket(ConnectSocket);
    WSACleanup();
}

int init_ui() {
    system("cls");
    cursor_show(0);

    printf("Choose a game mode:\n");
    printf("   Single player.\n");
    printf("   Multiplayer.\n");
    printf("   Quit.\n");
    int choose = 1;
    while (1) {
        cursor_go(0, choose);
        printf("->");
        cursor_go(0, choose);
        char input = (char) getch();
        switch (input) {
            case 'w':
                printf("  ");
                choose -= 1;
                if (choose == 0) {
                    choose = 3;
                }
                break;
            case 's':
                printf("  ");
                choose += 1;
                if (choose == 4) {
                    choose = 1;
                }
                break;
            case 13:
                if (choose == 1) {
                    single_player();
                    return 1;
                }
                if (choose == 2) {
                    multi_player();
                    return 1;
                }
                return 0;
            default:
                break;
        }
    }
}

int main() {
    while (init_ui()) {
    }
    return 0;
}