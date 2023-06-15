//
// Created by V on 2023/6/13.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "share.h"

Map map;
Snake player[2];
fd_set read_set;
sem_t mutex;
int conn_fd[2];

int openListenFd() {
    int listen_fd;
    struct sockaddr_in hints = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(DEFAULT_PORT)
    };

    //create listen socket
    if ((listen_fd = socket(hints.sin_family, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        return -1;
    }

    //bind listen socket
    if (bind(listen_fd, (struct sockaddr *) &hints, sizeof(hints)) == -1) {
        close(listen_fd);
        return -2;
    }
    return listen_fd;
}

void *sendThread() {
    pthread_detach(pthread_self());
    while (map.space && (player[0].current != dead || player[1].current != dead)) {
        for (int i = 0; i < 2; i += 1) {
            sem_wait(&mutex);
            moveSnake(&map, &player[i]);
            sem_post(&mutex);
        }
        for (int i = 0; i < 2; i += 1) {
            //send data to clients
            send(conn_fd[i], (char *) &map, sizeof(map), 0);
        }
        usleep(200000);
    }
    map.space = 0;
    for (int i = 0; i < 2; i += 1) {
        //send data to clients
        send(conn_fd[i], (char *) &map, sizeof(map), 0);
        close(conn_fd[i]);
    }
    return 0;
}

void *rcvThread() {
    int flag;
    int max_fd = conn_fd[0] > conn_fd[1] ? conn_fd[0] : conn_fd[1];
    char rcv_data;

    while (1) {
        FD_ZERO(&read_set);
        for (int i = 0; i < 2; i += 1) {
            FD_SET(conn_fd[i], &read_set);
        }
        flag = select(max_fd + 1, &read_set, 0, 0, 0);
        if (flag > 0) {
            for (int i = 0; i < 2; i += 1) {
                if (FD_ISSET(conn_fd[i], &read_set)) {
                    //receive data from client
                    recv(conn_fd[i], &rcv_data, 1, 0);
                    sem_wait(&mutex);
                    processInput(&player[i], rcv_data);
                    sem_post(&mutex);
                }
            }
        } else if (flag < 0) {
            break;
        }
    }
    return 0;
}

int main() {
    int listen_fd = openListenFd();
    
    switch (listen_fd) {
        case -1:
            printf("Failed to create listen socket.\n");
            printf("Press any key to continue");
            getchar();
            break;
        case -2:
            printf("Failed to bind listen socket.\n");
            printf("Press any key to continue");
            getchar();
            break;
        default:
            //start listening
            listen(listen_fd, 2);
            printf("Listening . . .\n");

            //start game
            while (1) {
                pthread_t send_thread, rcv_thread;

                printf("--------Starting a new game.--------\n");

                sem_init(&mutex, 0, 1);
                initMap(&map);
                initFood(&map);
                for (int i = 0; i < 2; i += 1) {
                    initSnake(&map, &player[i], Snake_Symbol[i]);
                }

                //create send thread
                pthread_create(&send_thread, 0, sendThread, 0);
                
                for (int i = 0; i < 2; i += 1) {
                    printf("Waiting for Player%d . . .\n", i + 1);
                    conn_fd[i] = accept(listen_fd, 0, 0);
                    printf("Connected.\n");
                }

                //create receive thread
                pthread_create(&rcv_thread, 0, rcvThread, 0);

                //wait for receive thread shutdown
                pthread_join(rcv_thread, 0);

                printf("Game over.\n");
            }
            //clean up
            close(listen_fd);
            break;
    }
    return 0;
}