#include "Maze.h"

bool removeItemFromMap(Room *room) {
  if (room != NULL || room->loot != NULL) {
    free(room->loot);
    room->loot = NULL;

    return true;
  }

  fprintf(stderr, "Could not remove item from map!\n");

  return false;
}

bool removeEnemyFromMap(Room *room) {
  if (room != NULL || room->enemy != NULL) {
    free(room->enemy);
    room->enemy = NULL;

    return true;
  }

  return false;
}
