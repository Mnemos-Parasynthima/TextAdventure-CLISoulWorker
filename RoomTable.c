#include <stdlib.h>
#include <stdio.h>

#include "Maze.h"
#include "Error.h"

#define ROOM_MULT 5 // By how much should room count increase

Table* initTable() {
  Table* table = (Table*) malloc(sizeof(Table));

  if (table == NULL) return NULL;

  table->rooms = (Room**) calloc(ROOM_MULT, sizeof(Room*));
  if (table->rooms == NULL) {
    free(table);
    return NULL;
  }

  table->cap = ROOM_MULT;
  table->len = 0;

  return table;
}

bool putRoom(Table* table, Room* room, bool overwrite) {
  char id = room->id;

  Room* _room = table->rooms[(int)id];

  if (_room == NULL) {
    if (id >= table->cap) { // Adding this item will increase the current array/size of the table
      // Increase the array by five more pointers
      table->rooms = (Room**) realloc(table->rooms, ROOM_MULT*sizeof(Room*));
      if (table->rooms == NULL) handleError(ERR_MEM, FATAL, "Could not reallocate space!\n");

      table->cap += ROOM_MULT;
    }

    table->rooms[(int)id] = room;
    table->len++;

    return true;
  } else if (overwrite) {
    // A room with the same id already exists. Use the new room and delete the overwritten roon.
    // Note that even though the rooms may have the same id, their contents (loot table, enemy table, etc) may be different.
    deleteRoom(room);

    table->rooms[(int)id] = _room;

    return true;
  }

  return false;
}

void deleteTable(Table* table) {
  if (table == NULL) return;

  free(table->rooms);
  free(table);
  table = NULL;
}

void addAndRecurse(Room* room, Table* table) {
  if (room != (void*)((long long)NO_EXIT)) {
    bool inTable = putRoom(table, room, false);

    // Prevent stack overflow by not going to rooms that have been inserted to the table
    //  although their exits may not be in table
    //  they'll be check by other paths
    // NOTE: Need to see behaviour for larger maps!!
    if (inTable) {
      for (int i = 0; i < 4; i++) {
        addAndRecurse(room->exits[i], table);
      }
    }
  }
}