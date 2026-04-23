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
  Node *head; // Head should always point to player!
  Node *tail;
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

CreatureList *init_creatures() {
  CreatureList *list = malloc(sizeof(CreatureList));

  Node *head = malloc(sizeof(Node));
  head->creature = initialize_player();
  head->next = NULL;
  head->prev = NULL;

  list->head = head;
  list->tail = head;

  return list;
}

void add_creature(CreatureList *list, Creature *creature) {
  Node *new_node = malloc(sizeof(Node));

  new_node->creature = creature;
  new_node->next = NULL;

  // if no head available then creature becomes head
  if (list->head == NULL) {
    list->head = new_node;
    list->tail = new_node;
    return;
  }
  // if head available then add as tail
  new_node->prev = list->tail;
  list->tail->next = new_node;
  list->tail = new_node;
}

Node *search_creature(CreatureList *list, Creature *creature) {
  if (list->head->creature == creature) { // check if head is our creature
    return list->head;
  }
  Node *current = list->head;
  while (current != NULL && current->creature != creature) {
    current = current->next;
  }
  return current;
}

// returns -1 if didn't delete because creature not found
// returns 0 if deleted
int delete_creature(CreatureList *list, Creature *creature) {
  Node *to_delete = search_creature(list, creature);
  if (to_delete == NULL) {
    return -1;
  }
  // if in middle
  if (to_delete->prev != NULL && to_delete->next != NULL) {
    to_delete->prev->next = to_delete->next;
    to_delete->next->prev = to_delete->prev;
  }
  // if is head
  if (to_delete->prev == NULL) {
    list->head = to_delete->next;
    list->head->prev = NULL;
  }
  // if is tail
  if (to_delete->next == NULL) {
    list->tail = to_delete->prev;
    list->tail->next = NULL;
  }
  free(to_delete);
  return 1;
}

Creature *get_creature(CreatureList *list, int idx) {
  Node *this = list->head;
  while (idx > 0 && this != NULL) {
    this = this->next;
    idx -= 1;
  }
  return this->creature;
}

Creature *at_coords(CreatureList *list, int x, int y) {
  Node *this = list->head;
  while (this != NULL) {
    if (this->creature->x == x && this->creature->y == y) {
      return this->creature;
    }
    this = this->next;
  }
  return NULL;
}
