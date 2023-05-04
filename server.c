//
// Created by V on 2023/4/26.
//
#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include "share.h"

char map[ROW * COL];
int apple[2];
snake player[2];
SOCKET ClientSocket[2];

DWORD WINAPI send_thread(LPVOID lpParameter) {
    int *flag = (int *) lpParameter;
    while (*flag) {
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

DWORD WINAPI player_thread(LPVOID lpParameter) {
    int *id = (int *) lpParameter;
    init_snake(map, playersymbol[*id], &player[*id]);
    char recvdata;
    while (1) {
        //receive data from client
        recv(ClientSocket[*id], &recvdata, 1, 0);

        process_input(recvdata, &player[*id]);
        if (player[*id].newdirection == QUIT_DIRECTION) {
            //player quit
            break;
        }
        if (player[*id].direction != INIT_DIRECTION) {
            move_snake(map, apple, &player[*id]);
        }
        if (player[*id].newdirection == DEAD_DIRECTION) {
            //player dead
            break;
        }
    }
    return 0;
}

int main() {
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
            int flag = 1;
            int playerid[2] = {0, 1};
            srand((unsigned) time(NULL));

            init_map(map);
            init_apple(map, apple);

            //create a thread for sending data
            CreateThread(NULL, 0, send_thread, &flag, 0, NULL);

            //create player threads
            for (int i = 0; i < 2; i++) {
                printf("Waiting for player%d...\n", i + 1);
                ClientSocket[i] = accept(ListenSocket, (LPSOCKADDR) &remoteaddr, &remoteaddrlen);
                printf("connected: %s\n", inet_ntoa(remoteaddr.sin_addr));
                playerthreads[i] = CreateThread(NULL, 0, player_thread, &playerid[i], 0, NULL);
            }

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