#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#define DEFAULT_HEALTH 10000

struct termios original_termios;

typedef struct {
    char name[64];
    int x, y, max_hp, hp, damage;
} Hero;

Hero* initialize_player();
void draw_frame(Hero* player, char input, int* is_running);
// TERMIOS
void disable_raw_mode();
void enable_raw_mode();



int main() {
    enable_raw_mode();

    int is_running = 1;
    Hero* player = initialize_player();
    while ( is_running ) {
        // timeout for input could be also implemented using VMIN
        char input = 0;
        read(STDIN_FILENO, &input, 1);
        draw_frame(player, input, &is_running);
    }
}


Hero* initialize_player() {
    Hero* player = malloc(sizeof(Hero));
    player->max_hp = DEFAULT_HEALTH;
    player->hp = DEFAULT_HEALTH;
    player->x = 30;
    player->y = 30;
    player->damage = 35;

    return player;
}

void moveY(int positions) {
    if (positions > 0) {
        printf("\x1b[%dA", positions);
    }
    else if (positions < 0) {
        printf("\x1b[%dB", positions);
    }
}

void moveX(int positions) {
    if (positions > 0) {
        printf("\x1b[%dC", positions);
    }
    else if (positions < 0) {
        printf("\x1b[%dD", positions);
    }
}

void moveTo(int row, int col) {
    printf("\x1b[%d;%df", row, col);
}

void draw_frame(Hero* player, char input, int* is_running) {
    printf("\x1b[2J");
    moveTo(1, 1);
    printf("q - exit");

    moveTo(player->y, player->x);
    printf("@");
    switch (input) {
        case 'a':
            player->x -= 1; 
            break;
        case 'd':
            player->x += 1; 
            break;
        case 'w':
            player->y -= 1; 
            break;
        case 's':
            player->y += 1; 
            break;
        case 'q':
            *is_running = 0;
            break;
    }

    fflush(stdout);
}

// TERMIOS
void disable_raw_mode() {
    printf("\x1b[?25h"); // enable cursor
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &original_termios);

    struct termios raw = original_termios;
    atexit(disable_raw_mode);

    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    printf("\x1b[?25l"); // disable cursor


    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
