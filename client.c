//
// Created by V on 2023/4/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

#define DEFAULT_PORT 19998

#define ROW 25
#define COL 50

typedef struct snakeinfo {
    int x[ROW], y[COL];
    int len, current_direction;
} snake;
const int shift[4][2] = {{-1, 0},
                         {0,  1},
                         {1,  0},
                         {0,  -1}};
int apple[2];
char map[ROW * COL];

void cursor_hide() {
    CONSOLE_CURSOR_INFO curInfo = {
            .dwSize = 1,
            .bVisible = FALSE
    };
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void cursor_go(short x, short y) {
    COORD pos = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void init_map() {
    for (short i = 0; i < ROW; i++) {
        for (short j = 0; j < COL; j++) {
            if (i == 0 || i == ROW - 1 || j == 0 || j == COL - 1) {
                map[i * COL + j] = '#';
                cursor_go(j, i);
                printf("%c", map[i * COL + j]);
            } else {
                map[i * COL + j] = ' ';
            }
        }
    }
}

void init_apple() {
    apple[0] = 0;
    apple[1] = 0;
    while (map[apple[0] * COL + apple[1]] != ' ') {
        apple[0] = rand() % ROW;
        apple[1] = rand() % COL;
    }
    map[apple[0] * COL + apple[1]] = '$';
}

void init_snake(snake *s) {
    s->current_direction = -10;
    s->len = 1;
    s->x[0] = 0;
    s->y[0] = 0;
    while (map[s->x[0] * COL + s->y[0]] != ' ') {
        s->x[0] = rand() % ROW;
        s->y[0] = rand() % COL;
    }
    map[s->x[0] * COL + s->y[0]] = '*';
}

void render_map() {
    for (short i = 1; i < ROW - 1; i++) {
        for (short j = 1; j < COL - 1; j++) {
            cursor_go(j, i);
            printf("%c", map[i * COL + j]);
        }
    }
}

void process_input(snake *s, int *d) {
    *d = -1;
    if (_kbhit()) {
        char tmp = (char) getch();
        switch (tmp) {
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
            case 'q':
                *d = 4;
                break;
            default:
                break;
        }
    }
    if (*d == -1 || abs(*d - s->current_direction) == 2) {
        return;
    }
    s->current_direction = *d;
}

void move_snake(snake *s, int *d) {
    int tmpx = s->x[s->len - 1], tmpy = s->y[s->len - 1];
    for (int i = s->len; i > 0; i--) {
        s->x[i] = s->x[i - 1];
        s->y[i] = s->y[i - 1];
    }
    s->x[0] += shift[s->current_direction][0];
    s->y[0] += shift[s->current_direction][1];
    if (map[s->x[0] * COL + s->y[0]] == '#' || map[s->x[0] * COL + s->y[0]] == '*') {
        *d = 5;
        return;
    }
    map[tmpx * COL + tmpy] = ' ';
    if (s->x[0] == apple[0] && s->y[0] == apple[1]) {
        s->len++;
        init_apple();
    }
    map[s->x[0] * COL + s->y[0]] = '*';
}

int init_ui() {
    system("cls");
    int ret;
    printf("choose a game mode(control:\"wasd\",quit in game:\"q\"):\n");
    printf("1.singleplayer\n");
    printf("2.multiplayer\n");
    printf("3.quit\n");
    scanf("%d", &ret);
    return ret;
}

void singleplayer() {
    system("cls");
    cursor_hide();
    snake player;
    int direction = -1;
    init_map();
    init_apple();
    init_snake(&player);
    render_map();
    while (1) {
        process_input(&player, &direction);
        if (direction == 4) {
            //player quit
            break;
        }
        if (player.current_direction != -10) {
            move_snake(&player, &direction);
        }
        if (direction == 5) {
            //player dead
            cursor_go(0, ROW);
            printf("game over\n");
            system("pause");
            break;
        }
        render_map();
        Sleep(200);
    }
}

void multiplayer() {
    system("cls");

    //init winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //creat a socket to connect
    SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    char targetip[20], senddata[1];
    while (1) {
        printf("enter the server ip address: ");
        scanf("%s", targetip);
        if (inet_addr(targetip) == INADDR_NONE) {
            printf("invalid ip address, try again.\n");
        } else {
            break;
        }
    }
    struct sockaddr_in hints;
    memset(&hints, 0, sizeof(hints));
    hints.sin_family = AF_INET;
    hints.sin_addr.s_addr = inet_addr(targetip);
    hints.sin_port = htons(DEFAULT_PORT);

    //connect to the server
    int iResult = connect(ConnectSocket, (SOCKADDR *) &hints, sizeof(hints));
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        printf("failed to connect to the server.");
        system("pause");
        return;
    }

    //connected
    cursor_hide();
    init_map();
    while (1) {
        //receive data from server
        recv(ConnectSocket, map, ROW * COL, 0);

        if (map[0] == 'r') {
            cursor_go(0, ROW);
            printf("game over\n");
            system("pause");
            break;
        }
        render_map();

        //keep sending data to server
        if (_kbhit()) {
            senddata[0] = (char) getch();
        } else {
            senddata[0] = 'k';
        }
        send(ConnectSocket, senddata, 1, 0);
    }

    //clean up
    closesocket(ConnectSocket);
    WSACleanup();
}

int main() {
    srand((unsigned) time(NULL));
    while (1) {
        int input = init_ui();
        if (input == 1) {
            singleplayer();
        } else if (input == 2) {
            multiplayer();
        } else if (input == 3) {
            break;
        }
    }
    return 0;
}