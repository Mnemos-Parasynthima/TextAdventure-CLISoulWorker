#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Setup.h"
#include "cJSON.h"

#define NO_EXIT 0xFEEDFAED

// TODO: Make own error handling/display

static char* readJSON() {
  FILE* file = fopen("./map.json", "r");

  if (file == NULL) {
    fprintf(stderr, "Could not open map!");
    exit(-1);
  }

  fseek(file, 0, SEEK_END);
  long len = ftell(file);
  rewind(file);

  char* buff = (char*) malloc(len + 1);
  if (buff == NULL) {
    fprintf(stderr, "Could not allocate memory for buffer!");
    exit(-1);
  }

  size_t read = fread(buff, 1, len, file);

  buff[len] = '\0';

  fclose(file);

  return buff;
}


static cJSON* readData() {
  char* buffer = readJSON();

  cJSON* json = cJSON_Parse(buffer);
  if (json == NULL) {
    const char* err = cJSON_GetErrorPtr();

    if (err != NULL) {
      fprintf(stderr, "Error: %s\n", err);
    }

    cJSON_Delete(json);    
  }

  cJSON_DeleteItemFromObjectCaseSensitive(json, "$schema");

  return json;
}

static void validateRoom(cJSON* room) {
  cJSON* isEntry = cJSON_GetObjectItemCaseSensitive(room, "isEntry");
  if (isEntry == NULL) {
    fprintf(stderr, "Room does not have isEntry data!");
    exit(-1);
  }
  if (isEntry->valueint != 1 && strcmp(room->valuestring, "0")) {
    fprintf(stderr, "The isEntry tag and room id do not match!"); // Better error text
    exit(-1);
  }

  cJSON* hasLoot = cJSON_GetObjectItemCaseSensitive(room, "hasLoot");
  if (hasLoot == NULL) {
    fprintf(stderr, "Room does not have hasLoot data!");
    exit(-1);
  }
  if (hasLoot->valueint < 0 || hasLoot->valueint > 1) {
    fprintf(stderr, "hasLoot must only be 1 for true or 0 for false!");;
    exit(-1);
  }

  cJSON* hasEnemies = cJSON_GetObjectItemCaseSensitive(room, "hasEnemies");
  if (hasEnemies == NULL) {
    fprintf(stderr, "Room does not have hasEnemies data!");
    exit(-1);
  }
  if (hasEnemies->valueint < 0 || hasEnemies->valueint > 1) {
    fprintf(stderr, "hasEnemies must only be 1 for true or 0 for false!");;
    exit(-1);
  }
  
  cJSON* info = cJSON_GetObjectItemCaseSensitive(room, "info");
  if (info == NULL) {
    fprintf(stderr, "Room does not have info data!");
    exit(-1);
  }

  cJSON* exits = cJSON_GetObjectItemCaseSensitive(room, "exits");
  if (exits == NULL) {
    fprintf(stderr, "Room does not have exits data!");
    exit(-1);
  }
  if (cJSON_GetArraySize(exits) != 4) {
    fprintf(stderr, "Room exits must only be 4!");
    exit(-1);
  }
  cJSON* e = NULL;
  cJSON_ArrayForEach(e, exits) {
    if (e->valueint < -1) {
      fprintf(stderr, "Exit markers cannot be less than -1!");

      exit(-1);
    }
  };

  cJSON* lootTable = cJSON_GetObjectItemCaseSensitive(room, "lootTable");
  if (lootTable == NULL) {
    fprintf(stderr, "Room does not have loot table data!");
    exit(-1);
  }
  // TODO: When a system of all possible loot items is implemented
  //    add a check to make sure all the items in the loot table are valid
  //  Do the same for enemies.

  cJSON* enemyTable = cJSON_GetObjectItemCaseSensitive(room, "enemyTable");
  if (enemyTable == NULL) {
    fprintf(stderr, "Room does not have enemy table data!");
    exit(-1);
  }
}


// Better param naming
static char* selectFromTable(int hasTable, cJSON* table) {
  if (hasTable == 0) return NULL;

  int len = cJSON_GetArraySize(table);
  int i = rand() % len;

  cJSON* item = cJSON_GetArrayItem(table, i);

  if (item == NULL) {
    fprintf(stderr, "Error! Could not get item!");
    exit(-1);
  }

  return item->valuestring;
}


static Room* createRoom(cJSON* _room) {
  Room* room = (Room*) malloc(sizeof(Room));

  cJSON* hasLoot = cJSON_GetObjectItemCaseSensitive(_room, "hasLoot");
  cJSON* hasEnemies = cJSON_GetObjectItemCaseSensitive(_room, "hasEnemies");
  cJSON* info = cJSON_GetObjectItemCaseSensitive(_room, "info");
  cJSON* exits = cJSON_GetObjectItemCaseSensitive(_room, "exits");
  cJSON* lootTable = cJSON_GetObjectItemCaseSensitive(_room, "lootTable");
  cJSON* enemyTable = cJSON_GetObjectItemCaseSensitive(_room, "enemyTable");


  room->id = (char) atoi(_room->string);

  room->info = (char*) malloc(sizeof(info->valuestring));
  strcpy(room->info, info->valuestring);

  char* loot = selectFromTable(hasLoot->valueint, lootTable);
  if (loot != NULL) {
    room->loot = (char*) malloc(sizeof(loot));
    strcpy(room->loot, loot);
  }

  char* enemy = selectFromTable(hasEnemies->valueint, enemyTable);
  if (enemy != NULL) {
    room->enemy = (char*) malloc(sizeof(enemy));
    strcpy(room->enemy, enemy);
  }

  cJSON* e = NULL;
  int i = 0;
  cJSON_ArrayForEach(e, exits) {
    if (e->valueint == -1) room->exits[i] = (void*) NO_EXIT;
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

Maze* initMaze() {
  cJSON* root = readData();

  // Quickly check if there is an entry room
  // Needs access to everything
  if (cJSON_GetObjectItemCaseSensitive(root, "0") == NULL) {
    fprintf(stderr, "There does not exist a room with value of '0' for the entry!");
    exit(-1);
  }

  cJSON* roomI = root->child;
  int i = 0; // ????
  int mazeSize = 0;

  Table* roomTable = initTable();

  while (roomI != NULL) {
    // Traversing through the rooms

    // Maybe validate each section, and if validated, add to the structure??
    // Instead of validating everything then getting/adding????
    validateRoom(roomI);
    Room* room = createRoom(roomI);
    mazeSize++;

    putRoom(roomTable, room);

    roomI = roomI->next;
  }

  // fprintf(stdout, "%d rooms have been created...", mazeSize);
  cJSON_Delete(root);

  // Time to connect the rooms
  Room* entry = connectRooms(roomTable);
  // Maybe a function to make sure all rooms have at least one connection???
  deleteTable(roomTable);

  Maze* maze = (Maze*) malloc(sizeof(Maze));
  if (maze == NULL) {
    fprintf(stderr, "Could not allocate maze!");
    exit(-1);
  }

  maze->entry = entry;
  maze->size = mazeSize;

  return maze;
}
