//
// Created by V on 2023/4/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <curses.h>
#include "share.h"

Map map;
char cache[ROW][COL];

void renderMap() {
    int score[2] = {0};

    for (int i = 0; i < ROW; i += 1) {
        for (int j = 0; j < COL; j += 1) {
            if (map.data[i][j] != cache[i][j]) {
                move(i, j);
                printw("%c", map.data[i][j]);
                cache[i][j] = map.data[i][j];
            }
            for (int k = 0; k < 2; k += 1) {
                if (map.data[i][j] == Snake_Symbol[k]) {
                    score[k] += 1;
                }
            }
        }
    }
    for (int i = 0; i < 2; i += 1) {
        if (score[i]) {
            move(3 + i, COL + 16);
            printw("%d", score[i] - 1);
        }
    }
    refresh();
}

void printInfo(int flag) {
    if (flag == 0) {
        move(ROW, 0);
        printw("Game over.\n");
        refresh();
    } else {
        move(0, COL + 1);
        printw("Control: \"wasd\"");
        move(1, COL + 1);
        printw("Quit: \"q\"");
        for (int i = 0; i < flag; i += 1) {
            move(i + 3, COL + 1);
            printw("Player%d score: ", i + 1);
        }
    }
}

void singlePlayer() {
    nodelay(stdscr, 1);
    clear();
    srand(time(0));

    Snake player;
    char input;

    initMap(&map);
    initFood(&map);
    initSnake(&map, &player, Snake_Symbol[0]);

    memset(cache, AIR, sizeof(cache));
    renderMap();
    printInfo(1);

    //start game
    while (map.space && player.current != dead) {
        input = getch();
        processInput(&player, input);
        moveSnake(&map, &player);
        renderMap();
        usleep(200000);
    }
    printInfo(0);
    getchar();
}

void multiPlayer() {
    clear();
    refresh();
    nodelay(stdscr, 1);
    echo();

    int clientfd;
    struct sockaddr_in hints;
    char server_ip[16];
    char send_data = 0;

    //create connect socket
    clientfd = socket(AF_INET, SOCK_STREAM, 0);

    //get server ip address
    while (1) {
        printw("Enter the server ip address: ");
        refresh();
        scanf("%s", server_ip);
        if (inet_addr(server_ip) == INADDR_NONE) {
            printw("Invalid ip address, try again.\n");
            refresh();
        } else {
            break;
        }
    }
    //connect to the server
    memset(&hints, 0, sizeof(hints));
    hints.sin_family = AF_INET;
    hints.sin_addr.s_addr = inet_addr(server_ip);
    hints.sin_port = htons(DEFAULT_PORT);
    if (connect(clientfd, (struct sockaddr *) &hints, sizeof(hints)) == -1) {
        close(clientfd);
        printw("Failed to connect to the server.\n");
        refresh();
        getchar();
        return;
    }

    //connected
    clear();
    noecho();

    memset(cache, AIR, sizeof(cache));
    printInfo(2);
    while (1) {
        //receive data from server
        recv(clientfd, (char *) &map, sizeof(map), 0);
        renderMap();

        if (!map.space) {
            shutdown(clientfd, SHUT_RDWR);
            break;
        }
        //send data to server
        send_data = getch();
        if (send_data) {
            send(clientfd, &send_data, 1, 0);
        }
        send_data = 0;
    }
    //clean up
    close(clientfd);

    printInfo(0);
    getchar();
}

int initUi() {
    noecho();
    clear();

    printw("Choose a game mode:\n");
    printw("   Single player.\n");
    printw("   Multiplayer.\n");
    printw("   Quit.\n");

    int choice = 1;
    char input;

    while (1) {
        move(choice, 0);
        printw("->");
        move(choice, 0);
        input = getch();
        switch (input) {
            case 'w':
                printw("  ");
                choice -= 1;
                if (choice == 0) {
                    choice = 3;
                }
                break;
            case 's':
                printw("  ");
                choice += 1;
                if (choice == 4) {
                    choice = 1;
                }
                break;
            case 'o':
                if (choice == 1) {
                    singlePlayer();
                    return 1;
                }
                if (choice == 2) {
                    multiPlayer();
                    return 1;
                }
                
                return 0;
            default:
                break;
        }
        refresh();
    }
}

int main() {
    initscr();
    cbreak();
    while (initUi()) {
    }
    endwin();
    return 0;
}