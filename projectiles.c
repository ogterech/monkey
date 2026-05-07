#include <math.h>
#include <stdlib.h>

typedef enum {
  ARROW_W,
  ARROW_E,
  ARROW_N,
  ARROW_S,
  ARROW_NW,
  ARROW_NE,
  ARROW_SW,
  ARROW_SE,
} ProjectileDirection;

typedef struct {
  int x, y;
  int vx, vy;
  int damage;
} Projectile;

// Doubly Linked list for projectiles
typedef struct PNode PNode;
struct PNode {
  PNode* next;
  PNode* prev;
  Projectile* projectile;
};

typedef struct {
  PNode* nil;
} ProjectileList;

// initializes empty ProjectileList
ProjectileList* alloc_projectiles() {
  ProjectileList* list = malloc(sizeof(ProjectileList));
  PNode* nil = malloc(sizeof(PNode));
  nil->next = nil;
  nil->prev = nil;
  list->nil = nil;
  return list;
}

void add_projectile(ProjectileList* list, Projectile* projectile) {
  PNode* new_node = malloc(sizeof(PNode));
  new_node->projectile = projectile;
  new_node->prev = list->nil->prev;
  new_node->next = list->nil;
  list->nil->prev->next = new_node;
  list->nil->prev = new_node;
}

PNode* search_projectile(ProjectileList* list, Projectile* projectile) {
  PNode* current = list->nil->next;
  while (current != list->nil && current->projectile != projectile) {
    current = current->next;
  }
  if (current == list->nil)
    return NULL;
  return current;
}

int delete_projectile(ProjectileList* list, Projectile* projectile) {
  PNode* to_delete = search_projectile(list, projectile);
  if (to_delete == NULL)
    return -1;
  to_delete->prev->next = to_delete->next;
  to_delete->next->prev = to_delete->prev;
  free(to_delete->projectile);
  free(to_delete);
  return 0;
}

Projectile* get_projectile(ProjectileList* list, int idx) {
  PNode* this = list->nil->next;
  while (idx > 0 && this != list->nil) {
    this = this->next;
    idx -= 1;
  }
  if (this == list->nil)
    return NULL;
  return this->projectile;
}

Projectile* projectile_at_coords(ProjectileList* list, int x, int y) {
  PNode* this = list->nil->next;
  while (this != list->nil) {
    if (this->projectile->x == x && this->projectile->y == y) {
      return this->projectile;
    }
    this = this->next;
  }
  return NULL;
}

// Derive direction from velocity
ProjectileDirection get_direction_from_velocity(int vx, int vy) {
  if (vx == -1 && vy == 0)
    return ARROW_W;
  if (vx == 1 && vy == 0)
    return ARROW_E;
  if (vx == 0 && vy == -1)
    return ARROW_N;
  if (vx == 0 && vy == 1)
    return ARROW_S;
  if (vx == -1 && vy == -1)
    return ARROW_NW;
  if (vx == 1 && vy == -1)
    return ARROW_NE;
  if (vx == -1 && vy == 1)
    return ARROW_SW;
  if (vx == 1 && vy == 1)
    return ARROW_SE;
  return ARROW_E;  // default
}

// Get character representation for arrow direction
char get_arrow_char(int vx, int vy) {
  ProjectileDirection dir = get_direction_from_velocity(vx, vy);
  switch (dir) {
    case ARROW_W:
      return '<';
    case ARROW_E:
      return '>';
    case ARROW_N:
      return '^';
    case ARROW_S:
      return 'v';
    case ARROW_NW:
      return '\\';
    case ARROW_NE:
      return '/';
    case ARROW_SW:
      return '/';
    case ARROW_SE:
      return '\\';
    default:
      return '*';
  }
}

// Calculate direction from archer to player
ProjectileDirection calculate_direction(int archer_x,
                                        int archer_y,
                                        int player_x,
                                        int player_y) {
  int dx = player_x - archer_x;
  int dy = player_y - archer_y;

  // Normalize to -1, 0, or 1
  int vx = (dx > 0) - (dx < 0);
  int vy = (dy > 0) - (dy < 0);

  if (vx == -1 && vy == 0)
    return ARROW_W;
  if (vx == 1 && vy == 0)
    return ARROW_E;
  if (vx == 0 && vy == -1)
    return ARROW_N;
  if (vx == 0 && vy == 1)
    return ARROW_S;
  if (vx == -1 && vy == -1)
    return ARROW_NW;
  if (vx == 1 && vy == -1)
    return ARROW_NE;
  if (vx == -1 && vy == 1)
    return ARROW_SW;
  if (vx == 1 && vy == 1)
    return ARROW_SE;

  // Default to east if no movement needed
  return ARROW_E;
}

// Get velocity components from direction
void get_direction_velocity(ProjectileDirection dir, int* vx, int* vy) {
  switch (dir) {
    case ARROW_W:
      *vx = -1;
      *vy = 0;
      break;
    case ARROW_E:
      *vx = 1;
      *vy = 0;
      break;
    case ARROW_N:
      *vx = 0;
      *vy = -1;
      break;
    case ARROW_S:
      *vx = 0;
      *vy = 1;
      break;
    case ARROW_NW:
      *vx = -1;
      *vy = -1;
      break;
    case ARROW_NE:
      *vx = 1;
      *vy = -1;
      break;
    case ARROW_SW:
      *vx = -1;
      *vy = 1;
      break;
    case ARROW_SE:
      *vx = 1;
      *vy = 1;
      break;
    default:
      *vx = 1;
      *vy = 0;
      break;
  }
}

// Create a new arrow projectile
Projectile* create_arrow(int x,
                         int y,
                         ProjectileDirection direction,
                         int damage) {
  Projectile* arrow = malloc(sizeof(Projectile));
  arrow->x = x;
  arrow->y = y;
  arrow->damage = damage;
  get_direction_velocity(direction, &arrow->vx, &arrow->vy);
  return arrow;
}

// Calculate distance squared between two points
int distance_squared(int x1, int y1, int x2, int y2) {
  int dx = x2 - x1;
  int dy = y2 - y1;
  return dx * dx + dy * dy;
}
