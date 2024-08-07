#include <stdlib.h>
#include <stdio.h>

#include "Setup.h"

Table* initTable() {
  Table* table = (Table*) malloc(sizeof(Table));

  if (table == NULL) return NULL;

  table->rooms = (Room**) calloc(5, sizeof(Room*));
  if (table->rooms == NULL) {
    free(table);
    return NULL;
  }

  table->cap = 5;
  table->len = 0;

  return table;
}

char putRoom(Table* table, Room* room) {
  char id = room->id;

  Room* _room = table->rooms[id];

  if (_room == NULL) {
    if (id >= table->cap) { // Adding this item will increase the current array/size of the table
      // Increase the array by five more pointers
      table->rooms = (Room**) realloc(table->rooms, 5*sizeof(Room*));
      table->cap += 5;
    }

    table->rooms[id] = room;
    table->len++;
  } else {
    // A room with the same id already exists. Default behaviour is to use the new room and delete the overwritten roon.
    // Note that even though the rooms may have the same id, their contents (loot table, enemy table, etc) may be different.
    deleteRoom(room);

    table->rooms[id] = _room;
  }

  return id;
}

void deleteTable(Table* table) {
  if (table == NULL) return;

  free(table->rooms);
  free(table);
}