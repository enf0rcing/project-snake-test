//
// Created by V on 2023/5/2.
//
#include <stdlib.h>
#include "share.h"

const char SnakeSymbol[2] = {"*o"};
const int shift[4][2] = {{-1, 0},
                         {0,  -1},
                         {1,  0},
                         {0,  1}};

void init_map(char *map) {
    for (int i = 0; i < ROW; i += 1) {
        for (int j = 0; j < COL; j += 1) {
            if (i == 0 || i == ROW - 1 || j == 0 || j == COL - 1) {
                map[i * COL + j] = WALL;
            } else {
                map[i * COL + j] = AIR;
            }
        }
    }
    map[ROW * COL] = 1;
}

void init_apple(char *map, int *apple) {
    do {
        apple[0] = rand() % ROW;
        apple[1] = rand() % COL;
    } while (map[apple[0] * COL + apple[1]] != AIR);
    map[apple[0] * COL + apple[1]] = APPLE;
}

void init_snake(char *map, char symbol, Snake *p) {
    p->len = 1;
    p->current = still;
    p->new = still;
    p->symbol = symbol;
    do {
        p->x[0] = rand() % ROW;
        p->y[0] = rand() % COL;
    } while (map[p->x[0] * COL + p->y[0]] != AIR);
    map[p->x[0] * COL + p->y[0]] = p->symbol;
}

void process_input(char input, Snake *p) {
    switch (input) {
        case 'w':
            p->new = up;
            break;
        case 'a':
            p->new = left;
            break;
        case 's':
            p->new = down;
            break;
        case 'd':
            p->new = right;
            break;
        case 'q':
            p->current = dead;
            return;
        default:
            return;
    }
    if (abs((int) (p->new - p->current)) == 2) {
        return;
    }
    p->current = p->new;
}

void move_snake(char *map, int *apple, Snake *p) {
    if (p->current == still || p->current == dead) {
        return;
    }
    int newHeadX = p->x[0] + shift[p->current][0], newHeadY = p->y[0] + shift[p->current][1];
    if (map[newHeadX * COL + newHeadY] == SnakeSymbol[0] || map[newHeadX * COL + newHeadY] == SnakeSymbol[1] ||
        map[newHeadX * COL + newHeadY] == WALL) {
        p->current = dead;
        return;
    }
    if (newHeadX == apple[0] && newHeadY == apple[1]) {
        p->len += 1;
        init_apple(map, apple);
    }
    map[p->x[p->len - 1] * COL + p->y[p->len - 1]] = AIR;
    for (int i = p->len; i > 0; i -= 1) {
        p->x[i] = p->x[i - 1];
        p->y[i] = p->y[i - 1];
    }
    p->x[0] = newHeadX;
    p->y[0] = newHeadY;
    map[p->x[0] * COL + p->y[0]] = p->symbol;
}