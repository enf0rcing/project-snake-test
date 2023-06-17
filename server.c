//
// Created by V on 2023/4/26.
//

#include "share.h"

#include <stdio.h>
#include <pthread.h>

#if defined (__WIN32__)
#include <winsock2.h>

#define Socket SOCKET
#define Sem HANDLE

typedef struct fd_set fd_set;

int close(Socket s) {
    return closesocket(s);
}
#elif __linux__
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define Socket int
#define Sem sem_t
#endif

void wait(unsigned int ms) {
    #if defined (__WIN32__)
    Sleep(ms);
    #elif __linux__
    usleep(1000 * ms);
    #endif
}

void initSem(Sem *s) {
    #if defined (__WIN32__)
    *s = CreateSemaphoreA(0, 1, 1, 0);
    #elif __linux__
    sem_init(s, 0, 1);
    #endif
}

void closeSem(Sem *s) {
    #if defined (__WIN32__)
    CloseHandle(*s);
    #elif __linux__
    sem_destroy(s);
    #endif
}

void P(Sem *s) {
    #if defined (__WIN32__)
    WaitForSingleObject(*s, INFINITE);
    #elif __linux__
    sem_wait(s);
    #endif
}

void V(Sem *s) {
    #if defined (__WIN32__)
    ReleaseSemaphore(*s, 1, 0);
    #elif __linux__
    sem_post(s);
    #endif
}

Map map;
Snake player[2];
Socket conn_socket[2];
fd_set read_set;
Sem mutex;

Socket openListenSocket() {
    Socket listen_socket;
    struct sockaddr_in hints = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(DEFAULT_PORT)
    };

    //create listen socket
    if ((listen_socket = socket(hints.sin_family, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        return -1;
    }

    //bind listen socket
    if (bind(listen_socket, (struct sockaddr *) &hints, sizeof(hints)) == -1) {
        close(listen_socket);
        return -2;
    }
    return listen_socket;
}

void *sendThread() {
    pthread_detach(pthread_self());

    while (map.space && (player[0].current != dead || player[1].current != dead)) {
        for (int i = 0; i < 2; i += 1) {
            P(&mutex);
            moveSnake(&map, &player[i]);
            V(&mutex);
            map.score[i] = player[i].len - 1;
        }
        for (int i = 0; i < 2; i += 1) {
            //send data to clients
            send(conn_socket[i], (char *) &map, sizeof(map), 0);
        }
        wait(200);
    }
    map.space = 0;
    for (int i = 0; i < 2; i += 1) {
        //send data to clients
        send(conn_socket[i], (char *) &map, sizeof(map), 0);
        close(conn_socket[i]);
    }
    return 0;
}

void *rcvThread() {
    int flag;
    int max = conn_socket[0] > conn_socket[1] ? conn_socket[0] : conn_socket[1];
    char rcv_data;

    while (1) {
        FD_ZERO(&read_set);
        for (int i = 0; i < 2; i += 1) {
            FD_SET(conn_socket[i], &read_set);
        }
        flag = select(max + 1, &read_set, 0, 0, 0);
        if (flag > 0) {
            for (int i = 0; i < 2; i += 1) {
                if (FD_ISSET(conn_socket[i], &read_set)) {
                    //receive data from client
                    recv(conn_socket[i], &rcv_data, 1, 0);

                    P(&mutex);
                    processInput(&player[i], rcv_data);
                    V(&mutex);
                }
            }
        } else if (flag < 0) {
            break;
        }
    }
    return 0;
}

int main() {
    #if defined (__WIN32__)
    WSADATA wsa_data;

    //init WinSock
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
    #endif
    
    Socket listen_socket;

    //create listen socket
    listen_socket = openListenSocket();

    switch (listen_socket) {
        case -1:
            printf("Failed to create listen socket.\n");
            printf("Press any key to continue.");
            getchar();
            break;
        case -2:
            printf("Failed to bind listen socket.\n");
            printf("Press any key to continue.");
            getchar();
            break;
        default:
            //start listening
            listen(listen_socket, 2);
            printf("Listening . . .\n");

            //start game
            while (1) {
                pthread_t send_thread, rcv_thread;

                printf("--------Starting a new game.--------\n");

                initSem(&mutex);
                initMap(&map);
                initFood(&map);
                for (int i = 0; i < 2; i += 1) {
                    initSnake(&map, &player[i], i);
                }

                //create send thread
                pthread_create(&send_thread, 0, sendThread, 0);

                for (int i = 0; i < 2; i += 1) {
                    printf("Waiting for Player%d . . .\n", i + 1);
                    conn_socket[i] = accept(listen_socket, 0, 0);
                    printf("Connected.\n");
                }

                //create rcv thread
                pthread_create(&rcv_thread, 0, rcvThread, 0);

                //wait for receive thread shutdown
                pthread_join(rcv_thread, 0);
                printf("Game over.\n");
            }
            //clean up
            closeSem(&mutex);
            close(listen_socket);
            #if defined (__WIN32__)
            WSACleanup();
            #endif
            break;
    }
    return 0;
}