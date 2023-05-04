//
// Created by V on 2023/4/26.
//
#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include "share.h"

int flag;
short apple[2];
char map[ROW * COL];
SOCKET ClientSocket[2];

DWORD WINAPI player1thread() {
    snake player1;
    init_snake(map, &player1, SNAKE_1);
    int direction1 = DEFAULT_DIRECTION;
    char recvdata1;
    while (1) {
        //receive data from client
        recv(ClientSocket[0], &recvdata1, 1, 0);

        process_input(&player1, recvdata1, &direction1);
        if (direction1 == QUIT_DIRECTION) {
            //player quit
            break;
        }
        if (player1.current_direction != INIT_DIRECTION) {
            move_snake(map, apple, &player1, SNAKE_1, &direction1);
        }
        if (direction1 == DEAD_DIRECTION) {
            //player dead
            break;
        }
    }
    return 0;
}

DWORD WINAPI player2thread() {
    snake player2;
    init_snake(map, &player2, SNAKE_2);
    int direction2 = DEFAULT_DIRECTION;
    char recvdata2;
    while (1) {
        //receive data from client
        recv(ClientSocket[1], &recvdata2, 1, 0);

        process_input(&player2, recvdata2, &direction2);
        if (direction2 == QUIT_DIRECTION) {
            //player quit
            break;
        }
        if (player2.current_direction != INIT_DIRECTION) {
            move_snake(map, apple, &player2, SNAKE_2, &direction2);
        }
        if (direction2 == DEAD_DIRECTION) {
            //player dead
            break;
        }
    }
    return 0;
}

DWORD WINAPI sendthread() {
    while (flag) {
        for (int i = 0; i < 2; i++) {
            //send data to clients
            send(ClientSocket[i], map, sizeof(map), 0);
        }
        Sleep(TIME_WAIT);
    }
    map[0] = RESTART;
    for (int i = 0; i < 2; i++) {
        //send data to clients
        send(ClientSocket[i], map, sizeof(map), 0);
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
    if (bind(ListenSocket, (LPSOCKADDR) &hints, sizeof(hints)) == SOCKET_ERROR) {
        printf("bind error\n");
        system("pause");
    } else {
        //start listening
        listen(ListenSocket, SOMAXCONN);
        printf("Listening...\n");

        //prepare to connections
        struct sockaddr_in remoteaddr;
        int remoteaddrlen = sizeof(remoteaddr);
        HANDLE playerthreads[2];

        //start game
        while (1) {
            printf("starting a new game...\n");
            flag = 1;
            init_map(map);
            init_apple(map, apple);

            //create a thread for sending data
            CreateThread(NULL, 0, sendthread, 0, 0, NULL);

            //if connected, create player1 thread
            printf("Waiting for player1...\n");
            ClientSocket[0] = accept(ListenSocket, (LPSOCKADDR) &remoteaddr, &remoteaddrlen);
            printf("connected: %s\n", inet_ntoa(remoteaddr.sin_addr));
            playerthreads[0] = CreateThread(NULL, 0, player1thread, 0, 0, NULL);

            //if connected, create player2 thread
            printf("Waiting for player2...\n");
            ClientSocket[1] = accept(ListenSocket, (LPSOCKADDR) &remoteaddr, &remoteaddrlen);
            printf("connected: %s\n", inet_ntoa(remoteaddr.sin_addr));
            playerthreads[1] = CreateThread(NULL, 0, player2thread, 0, 0, NULL);

            //wait for player threads shutdown
            WaitForMultipleObjects(2, playerthreads, TRUE, INFINITE);
            flag = 0;
            Sleep(TIME_WAIT);
        }
    }
    //clean up
    for (int i = 0; i < 2; i++) {
        closesocket(ClientSocket[i]);
    }
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}