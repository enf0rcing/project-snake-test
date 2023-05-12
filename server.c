//
// Created by V on 2023/4/26.
//
#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include "share.h"

char map[ROW * COL + 1];
int apple[2];
Snake player[2];
SOCKET ClientSocket[2];

DWORD WINAPI send_thread() {
    while (player[0].status != DEAD || player[1].status != DEAD) {
        for (int i = 0; i < 2; i += 1) {
            //send data to clients
            send(ClientSocket[i], map, sizeof(map), 0);
        }
        Sleep(100);
    }
    map[ROW * COL] = 0;
    for (int i = 0; i < 2; i += 1) {
        //send data to clients
        send(ClientSocket[i], map, sizeof(map), 0);
    }
    return 0;
}

DWORD WINAPI recv_thread(LPVOID lpParameter) {
    int *id = (int *) lpParameter;
    while (player[0].status != DEAD || player[1].status != DEAD) {
        //receive data from client
        char recvData;
        recv(ClientSocket[*id], &recvData, 1, 0);

        process_input(recvData, &player[*id]);
    }
    return 0;
}

DWORD WINAPI move_thread() {
    while (player[0].status != DEAD || player[1].status != DEAD) {
        for (int i = 0; i < 2; i += 1) {
            if (player[i].status != DEAD) {
                move_snake(map, apple, &player[i]);
            }
        }
        Sleep(200);
    }
    return 0;
}

int main() {
    //init WinSock
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
        printf("Bind error.\n");
        system("pause");
    } else {
        //start listening
        listen(ListenSocket, SOMAXCONN);
        printf("Listening . . .\n");

        //prepare to connections
        struct sockaddr_in remoteAddr;
        int remoteAddrLen = sizeof(remoteAddr);

        //start game
        int playerId[2] = {0, 1};
        HANDLE gameThread, sendThread, recvThreads[2];
        while (1) {
            printf("——————Starting a new game.——————\n");
            srand(time(0));

            init_map(map);
            init_apple(map, apple);
            for (int i = 0; i < 2; i += 1) {
                init_snake(map, SnakeSymbol[i], &player[i]);
            }

            //create send thread
            sendThread = CreateThread(0, 0, send_thread, 0, 0, 0);

            //create recv threads
            for (int i = 0; i < 2; i += 1) {
                printf("Waiting for Player%d . . .\n", i + 1);
                ClientSocket[i] = accept(ListenSocket, (LPSOCKADDR) &remoteAddr, &remoteAddrLen);
                printf("%s Connected.\n", inet_ntoa(remoteAddr.sin_addr));
                recvThreads[i] = CreateThread(0, 0, recv_thread, &playerId[i], 0, 0);
            }

            //create move thread
            gameThread = CreateThread(0, 0, move_thread, 0, 0, 0);

            //wait for move thread shutdown
            WaitForSingleObject(gameThread, INFINITE);

            //wait for send thread shutdown
            WaitForSingleObject(sendThread, INFINITE);

            printf("Game over.\n");

            //wait for recv threads shutdown
            WaitForMultipleObjects(2, recvThreads, 1, INFINITE);
        }

    }
    //clean up
    for (int i = 0; i < 2; i += 1) {
        closesocket(ClientSocket[i]);
    }
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}