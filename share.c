//
// Created by V on 2023/5/2.
//
#include <stdlib.h>
#include "share.h"

const char Snake_Symbol[2] = {"*o"};
const int shift[4][2] = {{-1, 0},
                         {0,  -1},
                         {1,  0},
                         {0,  1}};

void initMap(Map *map) {
    for (int i = 0; i < ROW; i += 1) {
        for (int j = 0; j < COL; j += 1) {
            if (i == 0 || i == ROW - 1 || j == 0 || j == COL - 1) {
                map->data[i][j] = WALL;
            } else {
                map->data[i][j] = AIR;
            }
        }
    }
    map->space = (ROW - 2) * (COL - 2);
}

void initFood(Map *map) {
    if (!map->space) {
        return;
    }
    map->space -= 1;
    int x, y;
    do {
        x = rand() % (ROW - 2) + 1;
        y = rand() % (COL - 2) + 1;
    } while (map->data[x][y] != AIR);
    map->data[x][y] = FOOD;
}

void initSnake(Map *map, Snake *p, char symbol) {
    if (!map->space) {
        return;
    }
    map->space -= 1;
    p->len = 1;
    p->current = still;
    p->new = still;
    p->symbol = symbol;
    do {
        p->x[0] = rand() % (ROW - 2) + 1;
        p->y[0] = rand() % (COL - 2) + 1;
    } while (map->data[p->x[0]][p->y[0]] != AIR);
    map->data[p->x[0]][p->y[0]] = p->symbol;
}

void processInput(Snake *p, char input) {
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

void moveSnake(Map *map, Snake *p) {
    if (p->current == still || p->current == dead) {
        return;
    }
    int newX = p->x[0] + shift[p->current][0];
    int newY = p->y[0] + shift[p->current][1];
    if (map->data[newX][newY] == Snake_Symbol[0] || map->data[newX][newY] == Snake_Symbol[1] || map->data[newX][newY] == WALL) {
        p->current = dead;
        return;
    }
    if (map->data[newX][newY] == FOOD) {
        p->len += 1;
        initFood(map);
    } else {
        map->data[p->x[p->len - 1]][p->y[p->len - 1]] = AIR;
    }
    for (int i = p->len; i > 0; i -= 1) {
        p->x[i] = p->x[i - 1];
        p->y[i] = p->y[i - 1];
    }
    p->x[0] = newX;
    p->y[0] = newY;
    map->data[newX][newY] = p->symbol;
}