#include <time.h>
#include <stdio.h>
#include <string.h>

#include "SaveLoad.h"
#include "cJSON.h"
#include "Error.h"
#include "Setup.h"


#define NO_ITEM 0x0
#define NAME "name"
#define HP "hp"
#define MAX_HP "maxHP"
#define INV_COUNT "invCount"
#define XP "xp"
#define ROOM "room"
#define INV "inventory"
#define ID "id"
#define MAP "map"
#define ITEM "item"
#define COUNT "count"

#define IS_ENTRY "isEntry"
#define INFO "info"
#define EXITS "exits"


// itoa is in assembly (just for fun), change it to C later
/**
 * Converts integer n into a character string.
 * @param n The integer
 * @param buffer The buffer to hold the string
 * @return The integer as a string
 */
extern char* itoa(int n, char* buffer);

static char* createError(cJSON* obj, const char* type) {
  cJSON_Delete(obj);

  handleError(ERR_DATA, ERROR, "Could not create JSON for %s!\n", type);

  return NULL;
}

/**
 * Recursively, adds the room to the table.
 * Note: Depending on size of maze, may or may not rework to avoid stack overflow
 * @param room The room to add
 * @param table The table to add to
 */
static void addAndRecurse(Room* room, Table* table) {
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


static char* createMapState() {
  Table* table = initTable();
  if (table == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for the table!\n");

  Room* room = maze->entry;
  addAndRecurse(room, table);
  room = NULL;

  // The table SHOULD have all of the rooms
  // Time to create the JSON object
  cJSON* mapObj = cJSON_CreateObject();
  if (mapObj == NULL) return createError(mapObj, "map");
  for (int i = 0; i < table->len; i++) {
    room = table->rooms[i];
    // Creating each room

    char buffer[2];
    char* idAsChar = itoa(room->id, buffer);

    cJSON* roomObj = cJSON_AddObjectToObject(mapObj, idAsChar);
    if (roomObj == NULL) return createError(roomObj, idAsChar);

    cJSON* isEntry = cJSON_AddNumberToObject(roomObj, IS_ENTRY, (room->id == 0) ? 1 : 0);
    if (isEntry == NULL) return createError(roomObj, IS_ENTRY);

    cJSON* info = cJSON_AddStringToObject(roomObj, INFO, room->info);
    if (info == NULL) return createError(roomObj, INFO);

    cJSON* exits = cJSON_AddArrayToObject(roomObj, EXITS);
    if (exits == NULL) return createError(roomObj, EXITS);
    for(int i = 0; i < 4; i++) {
      Room* roomExit = room->exits[i];
      // cJSON* exit;
      // if (roomExit == NO_ITEM) {
      //   exit = cJSON_CreateNumber(-1);
      // } else {
      //   exit = cJSON_CreateNumber(roomExit->id);
      // }
      cJSON* exit = cJSON_CreateNumber((roomExit == (void*)((long long)NO_EXIT) ? -1 : roomExit->id));
      if (exit == NULL) return createError(roomObj, "exit");

      if (!cJSON_AddItemToArray(exits, exit)) return createError(mapObj, "exit in exits\n");
    }
  }

  char* mapState = cJSON_Print(mapObj);

  cJSON_Delete(mapObj);
  deleteTable(table);

  return mapState;
}

static void saveMap() {
  const char* dir = "./data/saves/maps";

  char* mapState = createMapState();
  if (mapState == NULL) handleError(ERR_DATA, ERROR, "Could not create map state!\n");

    char filename[35];
    sprintf(filename, "%s/map_save.json", dir);

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
      handleError(ERR_IO, ERROR, "Unable to create the save!\n");
    } else {
      int written = fprintf(file, "%s", mapState);

      if (written <= 0) handleError(ERR_IO, ERROR, "Unable to save the map data!\n");
    }

    fclose(file);

    cJSON_free(mapState);
}

static char* createPlayerState() {
  cJSON* playerObj = cJSON_CreateObject();
  if (playerObj == NULL) return createError(playerObj, "player");

  cJSON* playerName = cJSON_AddStringToObject(playerObj, NAME, player->name);
  if (playerName == NULL) return createError(playerObj, NAME);

  cJSON* playerHP = cJSON_AddNumberToObject(playerObj, HP, player->hp);
  if (playerHP == NULL) return createError(playerObj, HP);

  cJSON* playerMaxHP = cJSON_AddNumberToObject(playerObj, MAX_HP, player->maxHP);
  if (playerMaxHP == NULL) return createError(playerObj, MAX_HP);

  cJSON* playerInvCount = cJSON_AddNumberToObject(playerObj, INV_COUNT, player->invCount);
  if (playerInvCount == NULL) return createError(playerObj, INV_COUNT);

  cJSON* playerXP = cJSON_AddNumberToObject(playerObj, XP, player->xp);
  if (playerXP == NULL) return createError(playerObj, XP);

  cJSON* playerRoom = cJSON_AddObjectToObject(playerObj, ROOM);
  if (playerRoom == NULL) return createError(playerObj, ROOM);
  cJSON* roomId = cJSON_AddNumberToObject(playerRoom, ID, player->room->id);
  if (roomId == NULL) return createError(playerObj, ID);
  cJSON* roomMap = cJSON_AddStringToObject(playerRoom, MAP, "../maps/map.json");
  if (roomMap == NULL) return createError(playerObj, MAP);

  cJSON* playerInv = cJSON_AddArrayToObject(playerObj, INV);
  if (playerInv == NULL) createError(playerObj, INV);
  for (int i = 0; i < INV_CAP; i++) {
    if (player->inv[i]._item != NO_ITEM) {
      cJSON* item = cJSON_CreateObject();
      if (item == NULL) return createError(playerObj, ITEM);

      cJSON* itemItem = cJSON_AddStringToObject(item, ITEM, (char*) &(player->inv[i]._item));
      if (itemItem == NULL) return createError(playerObj, "item name");

      cJSON* itemCount = cJSON_AddNumberToObject(item, COUNT, player->inv[i].count);
      if (itemCount == NULL) return createError(playerObj, COUNT);

      if(!cJSON_AddItemToArray(playerInv, item)) return createError(playerObj, "item in inventory");
    }
  }

  char* playerState = cJSON_Print(playerObj);

  cJSON_Delete(playerObj);
  return playerState;
}

static void savePlayer() {
  const char* dir = "./data/saves";

  char* playerState = createPlayerState();
  if (playerState == NULL) handleError(ERR_DATA, ERROR, "Could not create player state!\n");

  char filename[32];
  sprintf(filename, "%s/player_save.json", dir);

  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    handleError(ERR_IO, ERROR, "Unable to create the save!\n");
  } else {
    int written = fprintf(file, "%s", playerState);

    if (written <= 0) handleError(ERR_IO, ERROR, "Unable to save the player data!\n");
  }

  fclose(file);

  cJSON_free(playerState);
}

void saveGame() {
  savePlayer();
  saveMap();

  printf("The game has been saved!\n");
}

void loadGame(const char* savefile) {

}
