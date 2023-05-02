//
// Created by V on 2023/5/2.
//

#ifndef SNAKE_TEST_SHARE_H
#define SNAKE_TEST_SHARE_H

#endif //SNAKE_TEST_SHARE_H

#define DEFAULT_PORT 19998
#define ROW 25
#define COL 50
#define TIME_WAIT 200

typedef struct snakeinfo {
    int x[ROW * COL], y[ROW * COL];
    int len, current_direction;
} snake;

void init_map(char *);
void init_apple(char *, short *);
void init_snake(char *, snake *);
void process_input(snake *, char, int *);
void move_snake(char *, short *, snake*, int *);