#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define DEFAULT_HEALTH 10000

typedef enum {
  PLAYER,
  SKELETON_ARCHER,
} CreatureType;

typedef struct {
  char name[64];
  int x, y, max_hp, hp, damage;
  CreatureType type;
} Creature;

// Doubly Linked list for creatures
typedef struct Node Node;
struct Node {
  Node *next;
  Node *prev;
  Creature *creature;
};

typedef struct {
  Node *nil;
} CreatureList;

Creature *initialize_player() {
  Creature *player = malloc(sizeof(Creature));
  player->max_hp = DEFAULT_HEALTH;
  player->hp = DEFAULT_HEALTH;
  player->x = 30;
  player->y = 30;
  player->damage = 35;
  player->type = PLAYER;

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

  return skele;
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

Creature *at_coords(CreatureList *list, int x, int y) {
  Node *this = list->nil->next;
  while (this != list->nil) {
    if (this->creature->x == x && this->creature->y == y) {
      return this->creature;
    }
    this = this->next;
  }
  return NULL;
}
