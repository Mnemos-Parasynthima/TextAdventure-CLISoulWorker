#include <stdio.h>
#include <stdlib.h>

#include "Maze.h"
#include "Error.h"

Item* removeItemFromMap(Room *room) {
  if (room != NULL || room->loot != NULL) {
    Item* retItem = room->loot;
    room->loot = NULL;

    return retItem;
  }

  fprintf(stderr, "Could not remove item from map!\n"); // Add to error handling???

  return false;
}

bool deleteEnemyFromMap(Room *room) {
  if (room == NULL ||  room->enemy.enemy == NULL || room->enemy.boss == NULL) return false;

  if (room->hasBoss) return deleteBoss(room->enemy.boss);
  
  return deleteEnemy(room->enemy.enemy);
}