//
// Created by V on 2023/5/2.
//
#ifndef SNAKE_TEST_SHARE_H
#define SNAKE_TEST_SHARE_H

#define DEFAULT_PORT 23333
#define ROW 25
#define COL 50
#define TIME_WAIT 200

#define WALL '#'
#define AIR ' '
#define APPLE '$'

#define QUIT 'q'
#define RESTART 'r'
#define DEFAULT_INPUT 'k'

#define INIT_DIRECTION (-10)
#define DEFAULT_DIRECTION (-1)
#define QUIT_DIRECTION 4
#define DEAD_DIRECTION 5

typedef struct snakeInfo {
    int x[ROW * COL], y[ROW * COL];
    int len, direction, directionNew;
    char symbol;
} Snake;

extern const char playerSymbol[2];

void init_map(char *);

void init_apple(char *, int *);

void init_snake(char *, char, Snake *);

void process_input(char, Snake *);

void move_snake(char *, int *, Snake *);

#endif //SNAKE_TEST_SHARE_H

