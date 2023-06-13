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

#define INFO_DEAD 0
#define INFO_SINGLE 1
#define INFO_MULTI 2
#define INFO_UI 3

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
    switch (flag) {
        case INFO_DEAD:
            curs_set(1);
            nodelay(stdscr, 0);

            move(ROW, 0);
            printw("Game over.\n");
            printw("Press any key to continue.");
            break;
        case INFO_UI:
            noecho();
            curs_set(0);

            clear();
            printw("Choose a game mode:\n");
            printw("   Single player.\n");
            printw("   Multiplayer.\n");
            printw("   Quit.\n");
            break;
        default:
            noecho();
            curs_set(0);
            nodelay(stdscr, 1);

            clear();
            move(0, COL + 1);
            printw("Control: \"wasd\"");
            move(1, COL + 1);
            printw("Quit: \"q\"");
            for (int i = 0; i < flag; i += 1) {
                move(i + 3, COL + 1);
                printw("Player%d score: ", i + 1);
            }
            break;
    }
    refresh();
}

void singlePlayer() {
    Snake player;
    char input;

    srand(time(0));

    initMap(&map);
    initFood(&map);
    initSnake(&map, &player, Snake_Symbol[0]);

    memset(cache, AIR, sizeof(cache));
    printInfo(INFO_SINGLE);

    //start game
    while (map.space && player.current != dead) {
        input = getch();
        processInput(&player, input);
        moveSnake(&map, &player);
        renderMap();
        usleep(200000);
    }
    printInfo(INFO_DEAD);
    getchar();
}

void multiPlayer() {
    int clientfd;
    struct sockaddr_in hints;
    char server_ip[16];
    char send_data = 0;

    //create connect socket
    clientfd = socket(AF_INET, SOCK_STREAM, 0);

    //get server ip address
    echo();
    curs_set(1);
    clear();
    while (1) {
        printw("Enter the server ip address: ");
        refresh();
        scanw("%s", server_ip);
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
        printw("Failed to connect to the server.\n");
        printw("Press any key to continue.");
        refresh();
        getchar();
    } else {
        //connected
        memset(cache, AIR, sizeof(cache));
        printInfo(INFO_MULTI);
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
        printInfo(INFO_DEAD);
        getchar();
    }
    //clean up
    close(clientfd);
}

int renderMenu() {
    int choice = 1;
    char input;

    printInfo(INFO_UI);
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
            case ' ':
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
    while (renderMenu()) {
    }
    endwin();
    return 0;
}