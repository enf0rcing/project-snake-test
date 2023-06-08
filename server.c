//
// Created by V on 2023/4/26.
//
#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include "share.h"

Map map;
Snake player[2];
SOCKET client_socket[2];

DWORD WINAPI sendThread() {
    while (map.space && (player[0].current != dead || player[1].current != dead)) {
        for (int i = 0; i < 2; i += 1) {
            if (player[i].current != dead) {
                moveSnake(&map, &player[i]);
            }
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
        shutdown(client_socket[i], SD_BOTH);
    }
    return 0;
}

DWORD WINAPI rcvThread(LPVOID lp_param) {
    int id = *(int *) lp_param;
    char rcv_data;

    while (1) {
        //receive data from client
        recv(client_socket[id], &rcv_data, 1, 0);

        if (player[id].current != dead) {
            processInput(&player[id], rcv_data);
        } else {
            break;
        }
    }
    return 0;
}

int main() {
    WSADATA wsa_data;
    SOCKET listen_socket;
    struct sockaddr_in hints;
    int player_id[2] = {0, 1};

    //init WinSock
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    //create listen socket
    listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //bind listen socket
    memset(&hints, 0, sizeof(hints));
    hints.sin_family = AF_INET;
    hints.sin_addr.s_addr = INADDR_ANY;
    hints.sin_port = htons(DEFAULT_PORT);
    if (bind(listen_socket, (LPSOCKADDR) &hints, sizeof(hints)) == SOCKET_ERROR) {
        printf("Bind error.\n");
        system("pause");
    } else {
        //start listening
        listen(listen_socket, SOMAXCONN);
        printf("Listening . . .\n");

        //start game
        while (1) {
            printf("--------Starting a new game.--------\n");
            srand(time(0));

            HANDLE send_thread, rcv_thread[2];

            initMap(&map);
            initFood(&map);
            for (int i = 0; i < 2; i += 1) {
                initSnake(&map, &player[i], Snake_Symbol[i]);
            }

            //create send thread
            send_thread = CreateThread(0, 0, sendThread, 0, 0, 0);

            //create rcv threads
            for (int i = 0; i < 2; i += 1) {
                printf("Waiting for Player%d . . . ", i + 1);
                client_socket[i] = accept(listen_socket, 0, 0);
                printf("Connected.\n");
                rcv_thread[i] = CreateThread(0, 0, rcvThread, &player_id[i], 0, 0);
            }

            //wait for send thread shutdown
            WaitForSingleObject(send_thread, INFINITE);
            CloseHandle(send_thread);

            //wait for rcv threads shutdown
            WaitForMultipleObjects(2, rcv_thread, 1, INFINITE);
            for (int i = 0; i < 2; i += 1) {
                CloseHandle(rcv_thread[i]);
            }

            printf("Game over.\n");
        }
    }
    //clean up
    for (int i = 0; i < 2; i += 1) {
        closesocket(client_socket[i]);
    }
    closesocket(listen_socket);
    WSACleanup();

    return 0;
}