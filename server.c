//
// Created by V on 2023/4/26.
//
#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include "share.h"

Map map;
Snake player[2];
SOCKET ClientSocket[2];

DWORD WINAPI send_thread() {
    while (map.space && (player[0].current != dead || player[1].current != dead)) {
        for (int i = 0; i < 2; i += 1) {
            if (player[i].current != dead) {
                move_snake(&map, &player[i]);
            }
        }
        for (int i = 0; i < 2; i += 1) {
            //send data to clients
            send(ClientSocket[i], (char *) &map, sizeof(map), 0);
        }
        Sleep(200);
    }
    map.space = 0;
    for (int i = 0; i < 2; i += 1) {
        //send data to clients
        send(ClientSocket[i], (char *) &map, sizeof(map), 0);
    }
    return 0;
}

DWORD WINAPI recv_thread(LPVOID lpParameter) {
    int *id = (int *) lpParameter;
    while (1) {
        //receive data from client
        char recvData;
        recv(ClientSocket[*id], &recvData, 1, 0);

        if (player[*id].current != dead) {
            process_input(&player[*id], recvData);
        } else {
            break;
        }
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

        while (1) {
            printf("--------Starting a new game.--------\n");
            srand(time(0));
            HANDLE sendThread, recvThreads[2];

            init_map(&map);
            init_food(&map);
            for (int i = 0; i < 2; i += 1) {
                init_snake(&map, &player[i], SnakeSymbol[i]);
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

            //wait for send thread shutdown
            WaitForSingleObject(sendThread, INFINITE);
            CloseHandle(sendThread);

            //wait for recv threads shutdown
            WaitForMultipleObjects(2, recvThreads, 1, INFINITE);
            for (int i = 0; i < 2; i += 1) {
                CloseHandle(recvThreads[i]);
            }

            printf("Game over.\n");
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