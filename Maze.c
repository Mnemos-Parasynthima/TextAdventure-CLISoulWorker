#include <stdio.h>
#include <stdlib.h>

#include "Maze.h"
#include "Error.h"

void removeItemFromMap(Room *room) {
  if (room != NULL && room->loot != NULL) {
    free(room->loot);
    room->loot = NULL;

    return;
  }

  fprintf(stderr, "Could not remove item from map!\n"); // Add to error handling???

  return;
}

bool deleteEnemyFromMap(Room *room, bool deleteGear) {
  if (room == NULL ||  room->enemy.enemy == NULL || room->enemy.boss == NULL) return false;

  bool del;

  if (room->hasBoss) {
    del = deleteBoss(room->enemy.boss, deleteGear);
    room->enemy.boss = NULL; // For some reason, NULL applied in deleteBoss does not carry over????
    room->hasBoss = false;

    return del;
  }
  
  del = deleteEnemy(room->enemy.enemy);
  // Same thing, NULL doesn't carry over for some reason
  room->enemy.enemy = NULL;

  return del;
}

void deleteRoom(Room* room) {
  if (room->loot != NULL) deleteItem(room->loot);
  deleteEnemyFromMap(room, true);
  free(room->info);
  free(room);

  room = NULL;
}

void deleteMaze(Maze* maze) {
  Table* table = initTable();
  if (table == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for the table!\n");

  Room* room = maze->entry;
  addAndRecurse(room, table);
  room = NULL;

  for (int i = 0; i < table->len; i++) {
    deleteRoom(table->rooms[i]);
  }

  deleteTable(table);

  free(maze);
  maze = NULL;
}