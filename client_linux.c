//
// Created by V on 2023/6/13.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <curses.h>
#include "share.h"

#define INFO_UI 0
#define INFO_SINGLE 1
#define INFO_MULTI 2
#define INFO_DEAD 3

void renderMap(Map map, char cache[][COL]) {
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

void myPause() {
    printw("Press any key to continue.");
    refresh();
    getchar();
}

void printInfo(int flag) {
    switch (flag) {
        case INFO_DEAD:
            curs_set(1);
            nodelay(stdscr, 0);

            move(ROW, 0);
            printw("Game over.\n");
            myPause();
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
            printw("Control: 'w', 's', 'a', 'd'");
            move(1, COL + 1);
            printw("Quit: 'q'");
            for (int i = 0; i < flag; i += 1) {
                move(i + 3, COL + 1);
                printw("Player%d score: ", i + 1);
            }
            break;
    }
}

void singlePlayer() {
    Map map;
    Snake player;
    char cache[ROW][COL];
    char input;

    initMap(&map);
    initFood(&map);
    initSnake(&map, &player, Snake_Symbol[0]);
    memset(cache, AIR, sizeof(cache));
    printInfo(INFO_SINGLE);

    //start game
    while (map.space && player.current != dead) {
        input = (char) getch();
        if (input != -1) {
            processInput(&player, input);
        }
        moveSnake(&map, &player);
        renderMap(map, cache);
        usleep(200000);
    }
    printInfo(INFO_DEAD);
}

int openClientSock(char *host_name) {
    int client_fd;
    struct sockaddr_in hints = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(host_name),
        .sin_port = htons(DEFAULT_PORT)
    };

    //create client socket
    if ((client_fd = socket(hints.sin_family, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        return -1;
    }

    //connect to server
    if (connect(client_fd, (struct sockaddr *) &hints, sizeof(hints)) == -1) {
        close(client_fd);
        return -2;
    }
    return client_fd;
}

void multiPlayer() {
    Map map;
    int client_fd;
    struct in_addr tmp;
    char cache[ROW][COL] = {0};
    char host_name[16] = {0};
    char send_data;

    echo();
    curs_set(1);
    clear();
    while (host_name[0] != 'q') {
        //get server ip address
        printw("Enter the server ip address(Quit: 'q'): ");
        scanw("%s", host_name);
        if (inet_pton(AF_INET, host_name, &tmp) != 1) {
            printw("Invalid ip address, try again.\n");
        } else {
            printw("Connecting . . .\n");
            refresh();
            //create connect socket
            client_fd = openClientSock(host_name);
            switch (client_fd) {
                case -1:
                    printw("Failed to create client socket.\n");
                    myPause();
                    return;
                case -2:
                    printw("Failed to connect to the server.\n");
                    myPause();
                    return;
                default:    //connected
                    memset(cache, AIR, sizeof(cache));
                    printInfo(INFO_MULTI);
                    while (1) {
                        //receive data from server
                        recv(client_fd, (char *) &map, sizeof(map), 0);
                        renderMap(map, cache);
                        if (!map.space) {
                            close(client_fd);
                            break;
                        }

                        send_data = (char) getch();
                        if (send_data != -1) {
                            //send data to server
                            send(client_fd, &send_data, 1, 0);
                        }
                    }
                    printInfo(INFO_DEAD);
                    return;
            }
        }
    }
}

int renderMenu() {
    int choice = 1;
    char input;

    printInfo(INFO_UI);
    while (1) {
        move(choice, 0);
        printw("->");
        move(choice, 0);
        input = (char) getch();
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