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

  bool del;

  if (room->hasBoss) {
    del = deleteBoss(room->enemy.boss);
    room->enemy.boss = NULL; // For some reason, NULL applied in deleteBoss does not carry over????
    room->hasBoss = false;

    return del;
  }
  
  del = deleteEnemy(room->enemy.enemy);
  // Same thing, NULL doesn't carry over for some reason
  room->enemy.enemy = NULL;

  return del;
}