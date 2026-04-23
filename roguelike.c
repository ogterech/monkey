#include "creatures.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

struct terminalConfig {
  int rows, cols;
  struct termios original_termios;
};

struct terminalConfig config;
int debug = 0;
CreatureList *entities;

// GAME LOGIC
void draw_frame(time_t prev_time);
void process_input(char input, int *is_running);
void process_spawning(double diff_time);
// TERMIOS
void disable_raw_mode();
void enable_raw_mode();
void init_game();
int get_winsize(int *rows, int *cols);
void draw_line(int rows, int p);

int main(int argc, char **argv) {
  if (argc > 1 && strcmp("debug", argv[1]) == 0) {
    debug = 1;
  }
  init_game();

  int is_running = 1;
  time_t prev_time = time(NULL);
  while (is_running) {
    time_t new_time = time(NULL);
    double diff_time =
        difftime(new_time, prev_time); // elapsed_time is in seconds
    prev_time = new_time;

    char input = 0;
    read(STDIN_FILENO, &input, 1);
    process_input(input, &is_running);
    process_spawning(diff_time);
    draw_frame(prev_time);
  }
}

void moveY(int positions) {
  if (positions > 0) {
    printf("\x1b[%dA", positions);
  } else if (positions < 0) {
    printf("\x1b[%dB", positions);
  }
}

void moveX(int positions) {
  if (positions > 0) {
    printf("\x1b[%dC", positions);
  } else if (positions < 0) {
    printf("\x1b[%dD", positions);
  }
}

void moveTo(int row, int col) { printf("\x1b[%d;%df", row, col); }

void draw_frame(time_t prev_time) {
  printf("\x1b[2J");
  draw_line(0, 1);
  draw_line(0, config.cols - 1);
  draw_line(1, 1);
  draw_line(1, config.rows - 1);

  // Loop through all entities
  Node *creature_node = entities->head;
  while (creature_node != NULL) {
    Creature *creature = creature_node->creature;
    moveTo(creature->y, creature->x);
    switch (creature->type) {
    case PLAYER:
      printf("@");
      break;
    case SKELETON_ARCHER:
      printf("A");
      break;
    }
    creature_node = creature_node->next;
  }

  moveTo(1, 1);
  printf("q - exit\n\r");
  if (debug) {
    printf("ROWS %d\n\r", config.rows);
    printf("COLS %d\n\r", config.cols);
    printf("TIME %ld\n\r", prev_time);
  }

  fflush(stdout);
}

void process_input(char input, int *is_running) {
  Creature *player = entities->head->creature;

  int newx = player->x;
  int newy = player->y;
  switch (input) {
  case 'a':
    if (player->x > 2) {
      newx = player->x - 1;
    }
    break;
  case 'd':
    if (player->x < config.cols - 2) {
      newx = player->x + 1;
    }
    break;
  case 'w':
    if (player->y > 2) {
      newy = player->y - 1;
    }
    break;
  case 's':
    if (player->y < config.rows - 2) {
      newy = player->y + 1;
    }
    break;
  case 'q':
    *is_running = 0;
    break;
  }
  Creature *enemy = at_coords(entities, newx, newy);
  if (enemy != NULL && enemy != player) { // attack action here
    enemy->hp -= player->damage;
    if (enemy->hp <= 0) { // kill enemy
      delete_creature(entities, enemy);
    }
  } else {
    player->x = newx;
    player->y = newy;
  }
}

// TERMIOS
void disable_raw_mode() {
  printf("\x1b[?25h"); // enable cursor
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &config.original_termios);
}

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &config.original_termios);

  struct termios raw = config.original_termios;
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

int get_winsize(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_row == 0) {
    return -1;
  }
  *rows = ws.ws_row;
  *cols = ws.ws_col;
  return 0;
}

// if rows == 0 then we draw vertical line otherwise horizontal
void draw_line(int rows, int p) {
  // vertical
  if (rows == 0) {
    for (int i = 1; i < config.rows - 1; i++) {
      moveTo(i, p);
      printf("|");
    }
  } else { // horizontal
    for (int i = 1; i < config.cols - 1; i++) {
      moveTo(p, i);
      printf("–");
    }
  }
}

void init_game() {
  enable_raw_mode();
  if (get_winsize(&config.rows, &config.cols) != 0) {
    printf("Game doesn't work in your terminal :(");
    exit(1);
  }

  entities = malloc(sizeof(CreatureList));
  Creature *player = initialize_player();
  add_creature(entities, player);

  // seed the machine
  srand(time(NULL));
}

void process_spawning(double diff_time) {
  static double elapsed_time = 0.0;
  elapsed_time += diff_time;

  if (elapsed_time > 2.0) { // chance to spawn
    elapsed_time = 0.0;
    if (rand() % 4 == 0) {
      int x = 2 + rand() % (config.cols - 2);
      int y = 2 + rand() % (config.rows - 2);
      Creature *skeleton = initialize_skeleton(x, y);
      add_creature(entities, skeleton);
    }
  }
}
