//
// Created by V on 2023/4/26.
//
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#define DEFAULT_PORT 19998
#define ROW 25
#define COL 50

typedef struct snakeinfo {
    int x[ROW], y[COL];
    int len, current_direction;
} snake;
const int shift[4][2] = {{-1, 0},
                         {0,  1},
                         {1,  0},
                         {0,  -1}};
int apple[2];
char map[ROW * COL];
SOCKET ClientSocket[2];

void init_map() {
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            if (i == 0 || i == ROW - 1 || j == 0 || j == COL - 1) {
                map[i * COL + j] = '#';
            } else {
                map[i * COL + j] = ' ';
            }
        }
    }
}

void init_apple() {
    apple[0] = 0;
    apple[1] = 0;
    while (map[apple[0] * COL + apple[1]] != ' ') {
        apple[0] = rand() % ROW;
        apple[1] = rand() % COL;
    }
    map[apple[0] * COL + apple[1]] = '$';
}

void init_snake(snake *s) {
    s->current_direction = -10;
    s->len = 1;
    s->x[0] = 0;
    s->y[0] = 0;
    while (map[s->x[0] * COL + s->y[0]] != ' ') {
        s->x[0] = rand() % ROW;
        s->y[0] = rand() % COL;
    }
    map[s->x[0] * COL + s->y[0]] = '*';
}

void process_input(snake *s, char input, int *d) {
    *d = -1;
    switch (input) {
        case 'w':
            *d = 0;
            break;
        case 'd':
            *d = 1;
            break;
        case 's':
            *d = 2;
            break;
        case 'a':
            *d = 3;
            break;
        case 'q':
            *d = 4;
            break;
        default:
            break;
    }
    if (*d == -1 || abs(*d - s->current_direction) == 2) {
        return;
    }
    s->current_direction = *d;
}

void move_snake(snake *s, int *d) {
    map[s->x[s->len - 1] * COL + s->y[s->len - 1]] = ' ';
    for (int i = s->len; i > 0; i--) {
        s->x[i] = s->x[i - 1];
        s->y[i] = s->y[i - 1];
    }
    s->x[0] += shift[s->current_direction][0];
    s->y[0] += shift[s->current_direction][1];
    if (map[s->x[0] * COL + s->y[0]] == '#' || map[s->x[0] * COL + s->y[0]] == '*') {
        *d = 5;
        return;
    }
    if (s->x[0] == apple[0] && s->y[0] == apple[1]) {
        s->len++;
        init_apple();
    }
    map[s->x[0] * COL + s->y[0]] = '*';
}

DWORD WINAPI player1thread() {
    snake player1;
    init_snake(&player1);
    int direction1 = -1;
    char recvdata1[1];
    while (1) {
        //receive data from client
        recv(ClientSocket[0], recvdata1, 1, 0);
        process_input(&player1, recvdata1[0], &direction1);
        if (direction1 == 4) {
            //player quit
            break;
        }
        if (player1.current_direction != -10) {
            move_snake(&player1, &direction1);
        }
        if (direction1 == 5) {
            //player dead
            break;
        }
        Sleep(200);
    }
    return 0;
}

DWORD WINAPI player2thread() {
    snake player2;
    init_snake(&player2);
    int direction2 = -1;
    char recvdata2[1];
    while (1) {
        //receive data from client
        recv(ClientSocket[1], recvdata2, 1, 0);
        process_input(&player2, recvdata2[0], &direction2);
        if (direction2 == 4) {
            //player quit
            break;
        }
        if (player2.current_direction != -10) {
            move_snake(&player2, &direction2);
        }
        if (direction2 == 5) {
            //player dead
            break;
        }
        Sleep(200);
    }
    return 0;
}

DWORD WINAPI sendthread() {
    while (1) {
        for (int i = 0; i < 2; i++) {
            //send data to clients
            send(ClientSocket[i], map, sizeof(map), 0);
        }
        Sleep(200);
    }
    return 0;
}

int main() {
    srand((unsigned) time(NULL));
    //init winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    //create listen socket
    SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in hints;
    memset(&hints, 0, sizeof(hints));
    hints.sin_family = AF_INET;
    hints.sin_addr.s_addr = INADDR_ANY;
    hints.sin_port = htons(DEFAULT_PORT);
    //bind listen socket
    bind(ListenSocket, (LPSOCKADDR) &hints, sizeof(hints));
    //start listening
    printf("Listening...");
    listen(ListenSocket, SOMAXCONN);
    printf("success.\n");
    //prepare to connections
    struct sockaddr_in remoteaddr;
    int remoteaddrlen = sizeof(remoteaddr);
    HANDLE sthread, threads[2];
    init_map();
    init_apple();
    //create a thread for sending data
    sthread = CreateThread(NULL, 0, sendthread, 0, 0, NULL);
    //if connected, create player1 thread
    printf("Waiting for player1...\n");
    ClientSocket[0] = accept(ListenSocket, (LPSOCKADDR) &remoteaddr, &remoteaddrlen);
    printf("connected: %s\n", inet_ntoa(remoteaddr.sin_addr));
    threads[0] = CreateThread(NULL, 0, player1thread, 0, 0, NULL);
    //if connected, create player2 thread
    printf("Waiting for player2...\n");
    ClientSocket[1] = accept(ListenSocket, (LPSOCKADDR) &remoteaddr, &remoteaddrlen);
    printf("connected: %s\n", inet_ntoa(remoteaddr.sin_addr));
    threads[1] = CreateThread(NULL, 0, player2thread, 0, 0, NULL);
    //wait for player threads shutdown
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    //clean up
    for (int i = 0; i < 2; i++) {
        closesocket(ClientSocket[i]);
    }
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}