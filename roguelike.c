#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "creatures.c"
#include "projectiles.c"

struct terminalConfig {
  int rows, cols;
  struct termios original_termios;
};

struct terminalConfig config;
int debug = 0;
CreatureList* entities;
ProjectileList* projectiles;

typedef struct {
  int score;
  int skeletons_killed;
  int bananas_eaten;
  time_t game_start;
} GameStats;

GameStats stats = {0, 0, 0, 0};

// GAME LOGIC
void draw_frame(time_t prev_time);
void process_input(char input, int* is_running);
void process_spawning(double diff_time);
void process_creatures(time_t current_time);
void process_projectiles(int rows, int cols);
// TERMIOS
void disable_raw_mode();
void enable_raw_mode();
void init_game();
int get_winsize(int* rows, int* cols);
void draw_line(int rows, int p);
void moveTo(int row, int col);

int main(int argc, char** argv) {
  if (argc > 1 && strcmp("debug", argv[1]) == 0) {
    debug = 1;
  }
  init_game();

  int is_running = 1;
  time_t prev_time = time(NULL);
  Creature* player = entities->nil->next->creature;
  while (is_running) {
    time_t new_time = time(NULL);
    double diff_time =
        difftime(new_time, prev_time);  // elapsed_time is in seconds

    // Award 1 point per second survived
    if (new_time > prev_time) {
      stats.score += (int)(new_time - prev_time);
    }

    prev_time = new_time;

    char input = 0;
    read(STDIN_FILENO, &input, 1);
    process_input(input, &is_running);
    process_spawning(diff_time);
    process_creatures(new_time);
    process_projectiles(config.rows, config.cols);
    draw_frame(prev_time);

    // Check game over
    if (player->hp <= 0) {
      is_running = 0;
      printf("\x1b[2J");
      moveTo(config.rows / 2, config.cols / 2 - 10);
      printf("GAME OVER! Final Score: %d\n\r", stats.score);
      sleep(2);
    }
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

void moveTo(int row, int col) {
  printf("\x1b[%d;%df", row, col);
}

void draw_frame(time_t prev_time) {
  printf("\x1b[2J");
  draw_line(0, 1);
  draw_line(0, config.cols - 1);
  draw_line(1, 1);
  draw_line(1, config.rows - 1);

  Node* creature_node = entities->nil->next;
  while (creature_node != entities->nil) {
    Creature* creature = creature_node->creature;
    moveTo(creature->y, creature->x);
    switch (creature->type) {
      case PLAYER:
        printf("@");
        break;
      case SKELETON_ARCHER:
        printf("A");
        break;
      case BANANA:
        printf("b");
        break;
    }
    creature_node = creature_node->next;
  }

  // Draw projectiles (arrows)
  PNode* proj_node = projectiles->nil->next;
  while (proj_node != projectiles->nil) {
    Projectile* proj = proj_node->projectile;
    moveTo(proj->y, proj->x);
    printf("%c", get_arrow_char(proj->vx, proj->vy));
    proj_node = proj_node->next;
  }

  moveTo(1, 1);
  printf("q - exit\n\r");

  // Display score and statistics
  int time_survived = (int)difftime(prev_time, stats.game_start);
  moveTo(2, 1);
  printf("Score: %d | Time: %ds | Bananas: %d | Kills: %d\n\r", stats.score,
         time_survived, stats.bananas_eaten, stats.skeletons_killed);

  printf("Health: %d\n\r", entities->nil->next->creature->hp);

  if (debug) {
    printf("ROWS %d\n\r", config.rows);
    printf("COLS %d\n\r", config.cols);
    printf("TIME %ld\n\r", prev_time);
  }

  fflush(stdout);
}

void process_input(char input, int* is_running) {
  Creature* player = entities->nil->next->creature;

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
  Creature* target = at_coords(entities, newx, newy);
  Projectile* arrow = projectile_at_coords(projectiles, newx, newy);

  // Check if player steps into an arrow
  if (arrow != NULL) {
    player->hp -= arrow->damage;
    delete_projectile(projectiles, arrow);
  }

  if (target != NULL && target != player) {
    if (target->type == BANANA) {
      // Collect banana: heal and score
      if (player->hp < player->max_hp) {
        player->hp += 100;
        if (player->hp > player->max_hp) {
          player->hp = player->max_hp;
        }
      }
      stats.score += 5;
      stats.bananas_eaten++;
      delete_creature(entities, target);
      player->x = newx;
      player->y = newy;
    } else if (target->type == SKELETON_ARCHER) {
      // Attack enemy
      target->hp -= player->damage;
      if (target->hp <= 0) {
        stats.score += 30;
        stats.skeletons_killed++;
        delete_creature(entities, target);
      }
    }
  } else {
    player->x = newx;
    player->y = newy;
  }
}

// TERMIOS
void disable_raw_mode() {
  printf("\x1b[?25h");  // enable cursor
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
  printf("\x1b[?25l");  // disable cursor

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int get_winsize(int* rows, int* cols) {
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
  } else {  // horizontal
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

  entities = alloc_creatures();
  projectiles = alloc_projectiles();
  add_creature(entities, initialize_player());

  // Initialize game stats
  stats.score = 0;
  stats.skeletons_killed = 0;
  stats.bananas_eaten = 0;
  stats.game_start = time(NULL);

  // seed the machine
  srand(time(NULL));
}

void process_spawning(double diff_time) {
  static double elapsed_time = 0.0;
  elapsed_time += diff_time;

  if (elapsed_time > 2.0) {  // chance to spawn
    elapsed_time = 0.0;

    // Spawn skeletons: 1 in 4 chance
    if (rand() % 4 == 0) {
      int x = 2 + rand() % (config.cols - 2);
      int y = 2 + rand() % (config.rows - 2);
      Creature* skeleton = initialize_skeleton(x, y);
      add_creature(entities, skeleton);
    }

    // Spawn bananas: 1 in 8 chance, max 5
    if (rand() % 8 == 0 && count_bananas(entities) < 5) {
      int x = 2 + rand() % (config.cols - 2);
      int y = 2 + rand() % (config.rows - 2);
      Creature* banana = initialize_banana(x, y);
      add_creature(entities, banana);
    }
  }
}

// Process archer movement and shooting
void process_creatures(time_t current_time) {
  Creature* player = entities->nil->next->creature;
  if (player == NULL)
    return;

  Node* creature_node = entities->nil->next;
  while (creature_node != entities->nil) {
    Creature* creature = creature_node->creature;
    creature_node = creature_node->next;  // Advance before potential deletion

    if (creature->type != SKELETON_ARCHER)
      continue;

    int dist_sq = creature_distance_squared(creature, player);
    int can_shoot = can_archer_shoot(creature, current_time);

    int dx = player->x - creature->x;
    int dy = player->y - creature->y;
    int vx = (dx > 0) - (dx < 0);
    int vy = (dy > 0) - (dy < 0);

    int can_shoot_direction = (vx != 0 || vy != 0);

    if (can_shoot && can_shoot_direction) {
      ProjectileDirection dir =
          calculate_direction(creature->x, creature->y, player->x, player->y);
      Projectile* arrow =
          create_arrow(creature->x, creature->y, dir, creature->damage);
      add_projectile(projectiles, arrow);
      creature->last_skill_use = current_time;
    } else if (dist_sq <= 400) {
      // 50% chance to flee, 50% random walk
      if (rand() % 2 == 0) {
        move_archer_away(creature, player, config.rows, config.cols);
      } else {
        move_archer_random(creature, config.rows, config.cols);
      }
    } else {
      // Random walk when far away
      move_archer_random(creature, config.rows, config.cols);
    }

    // Check if archer moved into player (collision)
    if (creature->x == player->x && creature->y == player->y) {
      player->hp -= creature->damage;
    }
  }
}

// Process projectile movement and collisions
void process_projectiles(int rows, int cols) {
  Creature* player = entities->nil->next->creature;
  if (player == NULL)
    return;

  PNode* proj_node = projectiles->nil->next;
  while (proj_node != projectiles->nil) {
    Projectile* proj = proj_node->projectile;
    proj_node = proj_node->next;  // advance first, potential deletion

    int new_x = proj->x + proj->vx;
    int new_y = proj->y + proj->vy;

    // boundary collision
    if (new_x <= 1 || new_x >= cols - 1 || new_y <= 1 || new_y >= rows - 1) {
      // 50% chance to break, 50% chance to ricochet
      if (rand() % 2 == 0) {
        // break - delete arrow
        delete_projectile(projectiles, proj);
        continue;
      } else {
        // ricochet
        if (new_x <= 1 || new_x >= cols - 1) {
          proj->vx = -proj->vx;
        }
        if (new_y <= 1 || new_y >= rows - 1) {
          proj->vy = -proj->vy;
        }
        continue;
      }
    }

    // damage player
    if (new_x == player->x && new_y == player->y) {
      player->hp -= proj->damage;
      delete_projectile(projectiles, proj);
      continue;
    }

    // damage archer
    Node* creature_node = at_coords_node(entities, new_x, new_y);
    if (creature_node != NULL) {
      Creature* creature = creature_node->creature;
      if (creature->type == SKELETON_ARCHER && new_x == creature->x &&
          new_y == creature->y) {
        creature->hp -= proj->damage;
        if (creature->hp <= 0) {
          stats.skeletons_killed++;
          stats.score += 30;
          delete_creature(entities, creature);
        }
        delete_projectile(projectiles, proj);
      }
    }

    // If not deleted, update position
    if (search_projectile(projectiles, proj) != NULL) {
      proj->x = new_x;
      proj->y = new_y;
    }
  }
}
