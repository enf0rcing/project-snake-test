//
// Created by V on 2023/5/2.
//
#include <stdlib.h>
#include "share.h"

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

void init_apple(char *map, short *apple) {
    apple[0] = 0;
    apple[1] = 0;
    while (map[apple[0] * COL + apple[1]] != AIR) {
        apple[0] = rand() % ROW;
        apple[1] = rand() % COL;
    }
    map[apple[0] * COL + apple[1]] = APPLE;
}

void init_snake(char *map, snake *s, char symbol) {
    s->current_direction = INIT_DIRECTION;
    s->len = 1;
    s->x[0] = 0;
    s->y[0] = 0;
    while (map[s->x[0] * COL + s->y[0]] != AIR) {
        s->x[0] = rand() % ROW;
        s->y[0] = rand() % COL;
    }
    map[s->x[0] * COL + s->y[0]] = symbol;
}

void process_input(snake *s, char input, int *d) {
    *d = DEFAULT_DIRECTION;
    switch (input) {
        case 'w':
            *d = 0;
            break;
        case 'd':
            *d = 1;
            break;
        case 's':
            *d = 2;
            break;
        case 'a':
            *d = 3;
            break;
        case QUIT:
            *d = QUIT_DIRECTION;
            break;
        default:
            break;
    }
    if (*d == DEFAULT_DIRECTION || abs(*d - s->current_direction) == 2) {
        return;
    }
    s->current_direction = *d;
}

void move_snake(char *map, short *apple, snake *s, char symbol, int *d) {
    int tmpx = s->x[s->len - 1], tmpy = s->y[s->len - 1];
    for (int i = s->len; i > 0; i--) {
        s->x[i] = s->x[i - 1];
        s->y[i] = s->y[i - 1];
    }
    s->x[0] += shift[s->current_direction][0];
    s->y[0] += shift[s->current_direction][1];
    if (map[s->x[0] * COL + s->y[0]] == WALL || map[s->x[0] * COL + s->y[0]] == SNAKE_1 || map[s->x[0] * COL + s->y[0]] == SNAKE_2) {
        *d = DEAD_DIRECTION;
        return;
    }
    map[tmpx * COL + tmpy] = AIR;
    if (s->x[0] == apple[0] && s->y[0] == apple[1]) {
        s->len++;
        init_apple(map, apple);
    }
    map[s->x[0] * COL + s->y[0]] = symbol;
}