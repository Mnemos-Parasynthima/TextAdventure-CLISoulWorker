#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Error.h"
#include "Setup.h"
#include "LoadJSON.h"

#define MAPS_DIR "./data/maps";


char* readJSON(const char* filename) {
  FILE* file = fopen(filename, "r");

  if (file == NULL) handleError(ERR_IO, FATAL, "Could not open file!\n");

  fseek(file, 0, SEEK_END);
  long len = ftell(file);
  rewind(file);

  char* buff = (char*) malloc(len + 1);
  if (buff == NULL) handleError(ERR_MEM, FATAL, "Could not allocate memory for buffer!\n");

  fread(buff, 1, len, file);

  buff[len] = '\0';

  fclose(file);

  return buff;
}

cJSON* readData(const char* filename) {
  char* buffer = readJSON(filename);

  cJSON* json = cJSON_Parse(buffer);
  if (json == NULL) {
    const char* err = cJSON_GetErrorPtr();

    if (err != NULL) {
      fprintf(stderr, "Parsing error: %s\n", err);
    }

    cJSON_Delete(json);
  }

  cJSON_DeleteItemFromObjectCaseSensitive(json, "$schema");

  return json;
}

/**
 * Given a room, it validates its JSON data
 * @param room The cJSON structure to validate
 */
static void validateRoom(cJSON* room) {
  const char* dataErr = "Room %d: No %s data found!\n";

  char roomId = (char)atoi(room->string);

  cJSON* isEntry = cJSON_GetObjectItemCaseSensitive(room, "isEntry");
  if (isEntry == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "isEntry");
  if (strcmp(room->string, "0") == 0 && isEntry->valueint != 1) {
    handleError(ERR_DATA, FATAL, "Room %d: No matching isEntry data and room id!\n", roomId);
  }

  cJSON* info = cJSON_GetObjectItemCaseSensitive(room, "info");
  if (info == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "info");

  cJSON* exits = cJSON_GetObjectItemCaseSensitive(room, "exits");
  if (exits == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "exits");
  if (cJSON_GetArraySize(exits) != 4) handleError(ERR_DATA, FATAL, "Room %d: Exits must only be 4!\n", roomId);
  cJSON* e = NULL;
  cJSON_ArrayForEach(e, exits) {
    if (e->valueint < -1) handleError(ERR_DATA, FATAL, "Room %d: exit markers cannot be less than -1!\n", roomId);
  };

  cJSON* loot = cJSON_GetObjectItemCaseSensitive(room, "loot");
  if (loot == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "loot");
  // TODO: When a system of all possible loot items is implemented
  //    add a check to make sure all the items in the loot table are valid
  //  Do the same for enemies.

  cJSON* enemy = cJSON_GetObjectItemCaseSensitive(room, "enemy");
  if (enemy == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy");
}

/**
 * Selects something from the provided table, if any. Is random.
 * @param hasTable Whether there is a table to select from
 * @param table The table to select from
 * @return The selected object
 */
static char* selectFromTable(cJSON* table) {
  int len = cJSON_GetArraySize(table);

  if (len == 0) return NULL; // No items in table

  int i = rand() % len;

  cJSON* item = cJSON_GetArrayItem(table, i);

  if (item == NULL) {
    fprintf(stderr, "Error! Could not get item!\n");
    exit(-1);
  }

  return item->valuestring;
}

/**
 * Given a cJSON room, it creates a Room structure using its data
 * @param _room The cJSON room structure
 * @return The new Room
 */
static Room* createRoom(cJSON* _room) {
  Room* room = (Room*) malloc(sizeof(Room));
  room->enemy = NULL;
  room->loot = NULL;

  cJSON* info = cJSON_GetObjectItemCaseSensitive(_room, "info");
  cJSON* exits = cJSON_GetObjectItemCaseSensitive(_room, "exits");
  cJSON* lootTable = cJSON_GetObjectItemCaseSensitive(_room, "loot");
  cJSON* enemyTable = cJSON_GetObjectItemCaseSensitive(_room, "enemy");


  room->id = (char) atoi(_room->string);

  room->info = (char*) malloc(sizeof(char) * strlen(info->valuestring) + 1);
  strcpy(room->info, info->valuestring);

  char* loot = selectFromTable(lootTable);
  if (loot != NULL) {
    room->loot = (char*) malloc(sizeof(char) * strlen(loot) + 1);
    strcpy(room->loot, loot);
  }

  char* enemy = selectFromTable(enemyTable);
  if (enemy != NULL) {
    room->enemy = (char*) malloc(sizeof(char) * strlen(enemy) + 1);
    strcpy(room->enemy, enemy);
  }

  cJSON* e = NULL;
  int i = 0;
  cJSON_ArrayForEach(e, exits) {
    if (e->valueint == -1) room->exits[i] = (void*) ((long long)NO_EXIT);
    else room->exits[i] = (void*) ((long long) e->valueint);

    i++;
  };

  return room;
}

void deleteRoom(Room* room) {
  free(room->enemy);
  free(room->loot);
  free(room->info);
  free(room);

  room = NULL;
}

Room* connectRooms(Table* table) {
  Room *entry, *room;

  // Iterate though the table, getting each room
  for (int i = 0; i < table->len; i++) {
    room = table->rooms[i];

    if (room->id == 0) entry = room;
    
    // For a given room, iterate through its exits to connect
    for (int j = 0; j < 4; j++) {
      // Skip the ones that have no exit, keep the pointer to NO_EXIT
      if ((long long) room->exits[j] != NO_EXIT) {
        // Get the temp "address", aka the ids in terms of addresses/hex
        long long id = (long long)room->exits[j];

        room->exits[j] = table->rooms[id];
      }
    }
  }

  return entry;
}

Maze* initMaze(const char* filename) {
  cJSON* root = readData(filename);
  if (root == NULL) handleError(ERR_DATA, FATAL, "Could not parse JSON!\n");

  // Quickly check if there is an entry room
  // Needs access to everything
  if (cJSON_GetObjectItemCaseSensitive(root, "0") == NULL) {
    handleError(ERR_DATA, FATAL, "There does not exist a room with value of '0' for the entry!\n");
  }

  cJSON* roomI = root->child;
  int mazeSize = 0;

  Table* roomTable = initTable();
  if (roomTable == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for the table!\n");

  while (roomI != NULL) {
    // Traversing through the rooms

    // Maybe validate each section, and if validated, add to the structure??
    // Instead of validating everything then getting/adding????
    validateRoom(roomI);
    Room* room = createRoom(roomI);
    mazeSize++;

    putRoom(roomTable, room, true);

    roomI = roomI->next;
  }

  // fprintf(stdout, "%d rooms have been created...", mazeSize);
  cJSON_Delete(root);

  Room* entry = connectRooms(roomTable);
  // Maybe a function to make sure all rooms have at least one connection???
  deleteTable(roomTable);

  Maze* maze = (Maze*) malloc(sizeof(Maze));
  if (maze == NULL) handleError(ERR_MEM, FATAL, "Could not allocate maze!\n");

  maze->entry = entry;
  maze->size = mazeSize;

  return maze;
}