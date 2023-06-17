//
// Created by V on 2023/5/2.
//

#include "share.h"

#include <stdlib.h>
#include <time.h>

const char Snake_Symbol[2] = {"*o"};
const int shift[4][2] = {{-1, 0},
                         {0,  -1},
                         {1,  0},
                         {0,  1}};

#if defined (__WIN32__)
unsigned int rand_r(unsigned int *seed) {
    *seed = *seed * 1103515245 + 12345;
    return (unsigned int) (*seed / 65536) % 32768;
}
#endif

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
    for (int i = 0; i < 2; i += 1) {
        map->score[i] = 0;
    }
}

void initFood(Map *map) {
    if (!map->space) {
        return;
    }

    unsigned int x, y;
    unsigned int seed = time(0);

    do {
        x = rand_r(&seed) % (ROW - 2) + 1;
        y = rand_r(&seed) % (COL - 2) + 1;
    } while (map->data[x][y] != AIR);
    map->data[x][y] = FOOD;
    map->space -= 1;
}

void initSnake(Map *map, Snake *p, int flag) {
    if (!map->space) {
        return;
    }

    unsigned int seed = time(0);

    p->symbol = Snake_Symbol[flag];
    do {
        p->x[0] = rand_r(&seed) % (ROW - 2) + 1;
        p->y[0] = rand_r(&seed) % (COL - 2) + 1;
    } while (map->data[p->x[0]][p->y[0]] != AIR);
    map->data[p->x[0]][p->y[0]] = p->symbol;
    map->space -= 1;
    p->len = 1;
    p->current = still;
    p->next = still;
}

void processInput(Snake *p, char input) {
    if (p->current == dead) {
        return;
    }

    switch (input) {
        case 'w':
            p->next = up;
            break;
        case 'a':
            p->next = left;
            break;
        case 's':
            p->next = down;
            break;
        case 'd':
            p->next = right;
            break;
        case 'q':
            p->current = dead;
            return;
        default:
            return;
    }
    if (abs((int) (p->next - p->current)) == 2) {
        return;
    }
    p->current = p->next;
}

void moveSnake(Map *map, Snake *p) {
    if (p->current == still || p->current == dead) {
        return;
    }

    unsigned int newX = p->x[0] + shift[p->current][0];
    unsigned int newY = p->y[0] + shift[p->current][1];

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