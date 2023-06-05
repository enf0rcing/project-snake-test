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
#define APPLE '$'

#define Pos(x, y) ((x) * COL + y)

typedef enum status {
    up, left, down, right, still = 10, dead = -10
} Status;

typedef struct snakeInfo {
    int len, x[ROW * COL], y[ROW * COL];
    Status current, new;
    char symbol;
} Snake;

extern const char SnakeSymbol[2];

void init_map(char *);

void init_apple(char *, int *);

void init_snake(char *, char, Snake *);

void process_input(char, Snake *);

void move_snake(char *, int *, Snake *);

#endif //SNAKE_TEST_SHARE_H

