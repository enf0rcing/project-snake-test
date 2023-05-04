//
// Created by V on 2023/5/2.
//
#include <stdlib.h>
#include "share.h"

const char playersymbol[2] = {"*o"};
const int shift[4][2] = {{-1, 0},
                         {0,  1},
                         {1,  0},
                         {0,  -1}};

void init_map(char *map) {
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            if (i == 0 || i == ROW - 1 || j == 0 || j == COL - 1) {
                map[i * COL + j] = WALL;
            } else {
                map[i * COL + j] = AIR;
            }
        }
    }
}

void init_apple(char *map, int *apple) {
    do {
        apple[0] = rand() % ROW;
        apple[1] = rand() % COL;
    } while (map[apple[0] * COL + apple[1]] != AIR);
    map[apple[0] * COL + apple[1]] = APPLE;
}

void init_snake(char *map, char ps, snake *p) {
    p->len = 1;
    p->direction = INIT_DIRECTION;
    p->newdirection = DEFAULT_DIRECTION;
    p->symbol = ps;
    do {
        p->x[0] = rand() % ROW;
        p->y[0] = rand() % COL;
    } while (map[p->x[0] * COL + p->y[0]] != AIR);
    map[p->x[0] * COL + p->y[0]] = p->symbol;
}

void process_input(char input, snake *p) {
    switch (input) {
        case 'w':
            p->newdirection = 0;
            break;
        case 'd':
            p->newdirection = 1;
            break;
        case 's':
            p->newdirection = 2;
            break;
        case 'a':
            p->newdirection = 3;
            break;
        case QUIT:
            p->newdirection = QUIT_DIRECTION;
            break;
        default:
            p->newdirection = DEFAULT_DIRECTION;
            break;
    }
    if (p->newdirection == DEFAULT_DIRECTION || abs(p->newdirection - p->direction) == 2) {
        return;
    }
    p->direction = p->newdirection;
}

void move_snake(char *map, int *apple, snake *p) {
    int tmpx = p->x[p->len - 1], tmpy = p->y[p->len - 1];
    for (int i = p->len; i > 0; i--) {
        p->x[i] = p->x[i - 1];
        p->y[i] = p->y[i - 1];
    }
    p->x[0] += shift[p->direction][0];
    p->y[0] += shift[p->direction][1];
    if (map[p->x[0] * COL + p->y[0]] == playersymbol[0] || map[p->x[0] * COL + p->y[0]] == playersymbol[1] ||
        map[p->x[0] * COL + p->y[0]] == WALL) {
        p->newdirection = DEAD_DIRECTION;
        return;
    }
    map[tmpx * COL + tmpy] = AIR;
    if (p->x[0] == apple[0] && p->y[0] == apple[1]) {
        p->len++;
        init_apple(map, apple);
    }
    map[p->x[0] * COL + p->y[0]] = p->symbol;
}