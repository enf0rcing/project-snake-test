//
// Created by V on 2023/5/2.
//
#ifndef SNAKE_TEST_SHARE_H
#define SNAKE_TEST_SHARE_H

#define DEFAULT_PORT 23333
#define ROW 25
#define COL 50
#define SPACE ((ROW - 2) * (COL - 2))

#define WALL '#'
#define AIR ' '
#define FOOD '$'

typedef enum status {
    up, left, down, right, still = 10, dead = -10
} Status;

typedef struct map {
    char data[ROW][COL];
    int space;
} Map;

typedef struct snakeInfo {
    int len, x[SPACE], y[SPACE];
    Status current, new;
    char symbol;
} Snake;

extern const char SnakeSymbol[2];

void init_map(Map *);

void init_food(Map *);

void init_snake(Map *, Snake *, char);

void process_input(Snake *, char);

void move_snake(Map *, Snake *);

#endif //SNAKE_TEST_SHARE_H

