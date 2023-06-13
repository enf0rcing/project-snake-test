//
// Created by V on 2023/4/26.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "share.h"

Map map;
Snake player[2];
int connfd[2];

void *sendThread() {
    while (map.space && (player[0].current != dead || player[1].current != dead)) {
        for (int i = 0; i < 2; i += 1) {
            if (player[i].current != dead) {
                moveSnake(&map, &player[i]);
            }
        }
        for (int i = 0; i < 2; i += 1) {
            //send data to clients
            send(connfd[i], (char *) &map, sizeof(map), 0);
        }
        usleep(200000);
    }
    map.space = 0;
    for (int i = 0; i < 2; i += 1) {
        //send data to clients
        send(connfd[i], (char *) &map, sizeof(map), 0);
        shutdown(connfd[i], SHUT_RDWR);
    }
    return 0;
}

void *rcvThread(void *param) {
    int id = *((int *) param);
    char rcv_data;

    while (1) {
        //receive data from client
        recv(connfd[id], &rcv_data, 1, 0);

        if (player[id].current != dead) {
            processInput(&player[id], rcv_data);
        } else {
            break;
        }
    }
    return 0;
}

int main() {
    int listenfd;
    struct sockaddr_in hints;

    //create listen socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //bind listen socket
    memset(&hints, 0, sizeof(hints));
    hints.sin_family = AF_INET;
    hints.sin_addr.s_addr = INADDR_ANY;
    hints.sin_port = htons(DEFAULT_PORT);
    if (bind(listenfd, (struct sockaddr *) &hints, sizeof(hints)) == -1) {
        printf("Bind error.\n");
        getchar();
    } else {
        //start listening
        listen(listenfd, SOMAXCONN);
        printf("Listening . . .\n");

        //start game
        while (1) {
            printf("--------Starting a new game.--------\n");
            srand(time(0));

            pthread_t send_thread, rcv_thread[2];
            int player_id[2] = {0, 1};

            initMap(&map);
            initFood(&map);
            for (int i = 0; i < 2; i += 1) {
                initSnake(&map, &player[i], Snake_Symbol[i]);
            }

            //create send thread
            pthread_create(&send_thread, 0, sendThread, 0);

            //create rcv threads
            for (int i = 0; i < 2; i += 1) {
                printf("Waiting for Player%d . . . ", i + 1);
                connfd[i] = accept(listenfd, 0, 0);
                printf("Connected.\n");
                pthread_create(&rcv_thread[i], 0, rcvThread, &player_id[i]);
            }

            //wait for send thread shutdown
            pthread_join(send_thread, 0);

            //wait for rcv threads shutdown
            for (int i = 0; i < 2; i += 1) {
                pthread_join(rcv_thread[i], 0);
            }

            printf("Game over.\n");
        }
    }
    //clean up
    for (int i = 0; i < 2; i += 1) {
        close(connfd[i]);
    }
    close(listenfd);

    return 0;
}