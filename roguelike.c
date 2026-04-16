#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <termios.h>
#include <unistd.h>
#define DEFAULT_HEALTH 10000

struct termios original_termios;

typedef struct {
    char name[64];
    int x, y, max_hp, hp, damage;
} Hero;

Hero* initialize_player();
void draw_frame(Hero* player, char input);
// TERMIOS
void disable_raw_mode();
void enable_raw_mode();



int main() {
    enable_raw_mode();

    int is_running = 1;
    Hero* player = initialize_player();
    while ( is_running ) {
        char input = getchar();
        draw_frame(player, input);
    }
}


Hero* initialize_player() {
    Hero* player = malloc(sizeof(Hero));
    player->max_hp = DEFAULT_HEALTH;
    player->hp = DEFAULT_HEALTH;
    player->x = 0;
    player->y = 0;
    player->damage = 35;

    return player;
}

void draw_frame(Hero* player, char input) {
    printf("\x1btwojastara");
}

// TERMIOS
void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &original_termios);

  struct termios raw = original_termios;
  atexit(disable_raw_mode);

  raw.c_lflag &= ~(ECHO | ICANON);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
