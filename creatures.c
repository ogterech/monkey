#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define DEFAULT_HEALTH 500

typedef enum {
  PLAYER,
  SKELETON_ARCHER,
  BANANA,
} CreatureType;

typedef struct {
  char name[64];
  int x, y, max_hp, hp, damage;
  CreatureType type;
  time_t last_skill_use; // For archers: track when they last shot
} Creature;

// Doubly Linked list for creatures
typedef struct Node Node;
struct Node {
  Node *next;
  Node *prev;
  Creature *creature;
};

typedef struct {
  Node *nil; // nil->next->creature is always player
} CreatureList;

Creature *initialize_player(int x, int y) {
  Creature *player = malloc(sizeof(Creature));
  player->max_hp = DEFAULT_HEALTH;
  player->hp = DEFAULT_HEALTH;
  player->x = x;
  player->y = y;
  player->damage = 35;
  player->type = PLAYER;
  player->last_skill_use = 0;
  strcpy(player->name, "Player");

  return player;
}

Creature *initialize_skeleton(int x, int y) {
  Creature *skele = malloc(sizeof(Creature));
  int skele_hp = DEFAULT_HEALTH * 0.2;
  skele->max_hp = skele_hp;
  skele->hp = skele_hp;
  skele->x = x;
  skele->y = y;
  skele->damage = 35;
  skele->type = SKELETON_ARCHER;
  skele->last_skill_use = 0;
  strcpy(skele->name, "Skeleton Archer");

  return skele;
}

Creature *initialize_banana(int x, int y) {
  Creature *banana = malloc(sizeof(Creature));
  banana->max_hp = 1;
  banana->hp = 1;
  banana->x = x;
  banana->y = y;
  banana->damage = 0;
  banana->type = BANANA;
  banana->last_skill_use = 0;
  strcpy(banana->name, "Banana");

  return banana;
}

// initializes empty CreatureList
CreatureList *alloc_creatures() {
  CreatureList *list = malloc(sizeof(CreatureList));
  Node *nil = malloc(sizeof(Node));
  nil->next = nil;
  nil->prev = nil;
  list->nil = nil;
  return list;
}

void add_creature(CreatureList *list, Creature *creature) {
  Node *new_node = malloc(sizeof(Node));
  new_node->creature = creature;
  new_node->prev = list->nil->prev;
  new_node->next = list->nil;
  list->nil->prev->next = new_node;
  list->nil->prev = new_node;
}

Node *search_creature(CreatureList *list, Creature *creature) {
  Node *current = list->nil->next;
  while (current != list->nil && current->creature != creature) {
    current = current->next;
  }
  if (current == list->nil)
    return NULL;
  return current;
}

int delete_creature(CreatureList *list, Creature *creature) {
  Node *to_delete = search_creature(list, creature);
  if (to_delete == NULL)
    return -1;
  to_delete->prev->next = to_delete->next;
  to_delete->next->prev = to_delete->prev;
  free(to_delete);
  return 0;
}

Creature *get_creature(CreatureList *list, int idx) {
  Node *this = list->nil->next;
  while (idx > 0 && this != list->nil) {
    this = this->next;
    idx -= 1;
  }
  if (this == list->nil)
    return NULL;
  return this->creature;
}

Node *at_coords_node(CreatureList *list, int x, int y) {
  Node *this = list->nil->next;
  while (this != list->nil) {
    if (this->creature->x == x && this->creature->y == y) {
      return this;
    }
    this = this->next;
  }
  return NULL;
}

Creature *at_coords(CreatureList *list, int x, int y) {
  Node *node = at_coords_node(list, x, y);
  if (node == NULL)
    return NULL;
  return node->creature;
}

int count_bananas(CreatureList *list) {
  int count = 0;
  Node *current = list->nil->next;
  while (current != list->nil) {
    if (current->creature->type == BANANA) {
      count++;
    }
    current = current->next;
  }
  return count;
}

// Calculate distance squared between two creatures
int creature_distance_squared(Creature *c1, Creature *c2) {
  int dx = c2->x - c1->x;
  int dy = c2->y - c1->y;
  return dx * dx + dy * dy;
}

// Move archer randomly (random walk)
void move_archer_random(Creature *archer, int rows, int cols) {
  // 8 possible directions
  int dx = (rand() % 3) - 1; // -1, 0, 1
  int dy = (rand() % 3) - 1; // -1, 0, 1

  // Ensure we don't stand still
  while (dx == 0 && dy == 0) {
    dx = (rand() % 3) - 1;
    dy = (rand() % 3) - 1;
  }

  int new_x = archer->x + dx;
  int new_y = archer->y + dy;

  // Check boundaries
  if (new_x > 1 && new_x < cols - 2 && new_y > 1 && new_y < rows - 2) {
    archer->x = new_x;
    archer->y = new_y;
  }
}

// Move archer away from player (flee behavior)
void move_archer_away(Creature *archer, Creature *player, int rows, int cols) {
  int dx = archer->x - player->x; // Opposite direction from player
  int dy = archer->y - player->y;

  // Normalize to -1, 0, or 1
  int move_x = (dx > 0) - (dx < 0);
  int move_y = (dy > 0) - (dy < 0);

  int new_x = archer->x + move_x;
  int new_y = archer->y + move_y;

  // Check boundaries
  if (new_x > 1 && new_x < cols - 2 && new_y > 1 && new_y < rows - 2) {
    archer->x = new_x;
    archer->y = new_y;
  }
}

// Check if archer can shoot (10 second cooldown)
int can_archer_shoot(Creature *archer, time_t current_time) {
  return (current_time - archer->last_skill_use) >= 10;
}
