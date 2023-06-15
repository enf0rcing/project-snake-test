//
// Created by V on 2023/4/26.
//

#include <stdio.h>
#include <pthread.h>
#include <winsock2.h>
#include "share.h"

Map map;
Snake player[2];
struct fd_set read_set;
HANDLE mutex;
SOCKET client_socket[2];

SOCKET openListenSock() {
    SOCKET listen_socket;
    struct sockaddr_in hints = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(DEFAULT_PORT)
    };

    //create listen socket
    if ((listen_socket = socket(hints.sin_family, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        return -1;
    }

    //bind listen socket
    if (bind(listen_socket, (struct sockaddr *) &hints, sizeof(hints)) == SOCKET_ERROR) {
        closesocket(listen_socket);
        return -2;
    }
    return listen_socket;
}

void* sendThread() {
    pthread_detach(pthread_self());
    while (map.space && (player[0].current != dead || player[1].current != dead)) {
        for (int i = 0; i < 2; i += 1) {
            WaitForSingleObject(mutex, INFINITE);
            moveSnake(&map, &player[i]);
            ReleaseSemaphore(mutex, 1, 0);
        }
        for (int i = 0; i < 2; i += 1) {
            //send data to clients
            send(client_socket[i], (char *) &map, sizeof(map), 0);
        }
        Sleep(200);
    }
    map.space = 0;
    for (int i = 0; i < 2; i += 1) {
        //send data to clients
        send(client_socket[i], (char *) &map, sizeof(map), 0);
        closesocket(client_socket[i]);
    }
    return 0;
}

void* rcvThread() {
    int flag;
    char rcv_data;

    while (1) {
        FD_ZERO(&read_set);
        for (int i = 0; i < 2; i += 1) {
            FD_SET(client_socket[i], &read_set);
        }
        flag = select(0, &read_set, 0, 0, 0);
        if (flag > 0) {
            for (int i = 0; i < 2; i += 1) {
                if (FD_ISSET(client_socket[i], &read_set)) {
                    //receive data from client
                    recv(client_socket[i], &rcv_data, 1, 0);

                    WaitForSingleObject(mutex, INFINITE);
                    processInput(&player[i], rcv_data);
                    ReleaseSemaphore(mutex, 1, 0);
                }
            }
        } else if (flag < 0) {
            break;
        }
    }
    return 0;
}

int main() {
    WSADATA wsa_data;
    SOCKET listen_socket;

    //init WinSock
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    //create listen socket
    listen_socket = openListenSock();

    switch (listen_socket) {
        case -1:
            printf("Failed to create listen socket.\n");
            system("pause");
            break;
        case -2:
            printf("Failed to bind listen socket.\n");
            system("pause");
            break;
        default:
            //start listening
            listen(listen_socket, 2);
            printf("Listening . . .\n");

            //start game
            while (1) {
                pthread_t send_thread, rcv_thread;

                printf("--------Starting a new game.--------\n");

                mutex = CreateSemaphore(0, 1, 1, 0);
                initMap(&map);
                initFood(&map);
                for (int i = 0; i < 2; i += 1) {
                    initSnake(&map, &player[i], Snake_Symbol[i]);
                }

                //create send thread
                pthread_create(&send_thread, 0, sendThread, 0);

                for (int i = 0; i < 2; i += 1) {
                    printf("Waiting for Player%d . . .\n", i + 1);
                    client_socket[i] = accept(listen_socket, 0, 0);
                    printf("Connected.\n");
                }

                //create rcv thread
                pthread_create(&rcv_thread, 0, rcvThread, 0);

                //wait for receive thread shutdown
                pthread_join(rcv_thread, 0);

                printf("Game over.\n");
            }
            //clean up
            CloseHandle(mutex);
            closesocket(listen_socket);
            WSACleanup();
            break;
    }
    return 0;
}