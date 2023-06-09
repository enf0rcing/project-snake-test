//
// Created by V on 2023/5/2.
//

#ifndef SNAKE_TEST_SHARE_H
#define SNAKE_TEST_SHARE_H

#define DEFAULT_PORT 23333
#define ROW 25
#define COL 50

#define WALL '#'
#define AIR ' '
#define FOOD '$'

enum status {
    up, left, down, right, still = 10, dead = -10
};

typedef struct map_info {
    char data[ROW][COL];
    unsigned int space;
    unsigned int score[2];
} Map;

typedef struct snake_info {
    char symbol;
    enum status current;
    unsigned int len;
    unsigned int x[(ROW - 2) * (COL - 2)], y[(ROW - 2) * (COL - 2)];
} Snake;

void initMap(Map *map);

void initFood(Map *map);

void initSnake(Map *map, Snake *p, int flag);

void processInput(Snake *p, char input);

void moveSnake(Map *map, Snake *p);

#endif //SNAKE_TEST_SHARE_H