#include <time.h>
#include <stdio.h>
#include <string.h>

#include "SaveLoad.h"
#include "LoadJSON.h"
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
#define LOOT "loot"
#define ENEMY "enemy"

const char* SAVE_DIR = "./data/saves";

// itoa is in assembly (just for fun), change it to C later
/**
 * Converts integer n into a character string.
 * @param n The integer
 * @param buffer The buffer to hold the string
 * @return The integer as a string
 */
extern char* itoa(int n, char* buffer);

/**
 * Handles errors regarding cJSON objects.
 * @param obj The cJSON object
 * @param type The JSON object tag
 * @return NULL
 */
static char* createError(cJSON* obj, const char* type) {
  cJSON_Delete(obj);

  handleError(ERR_DATA, ERROR, "Could not create JSON for %s!\n", type);

  return NULL;
}

/**
 * Finds a room with the given ID.
 * @param room The room to look at
 * @param id The target ID
 * @return Room with matching ID
 */
static Room* findRoom(Room* room, char id) {
  if (room != (void*)((long long)NO_EXIT)) {
    if (room->id == id) return room;

    for (int i = 0; i < 4; i++) {
      Room* target = findRoom(room->exits[i], id);
      if (target != (void*)((long long)NO_EXIT)) return target;
    }
  }

  return (void*)((long long)NO_EXIT);
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

/**
 * Creates the maze state for saving.
 * @return The maze as a JSON string
 */
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
    if (roomObj == NULL) return createError(mapObj, idAsChar);

    cJSON* isEntry = cJSON_AddNumberToObject(roomObj, IS_ENTRY, (room->id == 0) ? 1 : 0);
    if (isEntry == NULL) return createError(mapObj, IS_ENTRY);

    cJSON* info = cJSON_AddStringToObject(roomObj, INFO, room->info);
    if (info == NULL) return createError(mapObj, INFO);

    cJSON* exits = cJSON_AddArrayToObject(roomObj, EXITS);
    if (exits == NULL) return createError(mapObj, EXITS);
    for (int i = 0; i < 4; i++) {
      Room* roomExit = room->exits[i];

      cJSON* exit = cJSON_CreateNumber((roomExit == (void*)((long long)NO_EXIT) ? -1 : roomExit->id));
      if (exit == NULL) return createError(mapObj, "exit");

      if (!cJSON_AddItemToArray(exits, exit)) return createError(mapObj, "exit in exits");
    }


    // Explore usage with CreateStringArray
    // Create the arrays just to be in compliance to format
    // But no need to add when none are present

    cJSON* loot = cJSON_AddArrayToObject(roomObj, LOOT);
    if (loot == NULL) return createError(mapObj, LOOT);
    if (room->loot != NULL) {
      cJSON* lootItem = cJSON_CreateString(room->loot);
      if (lootItem == NULL) return createError(mapObj, "loot item");
      if (!cJSON_AddItemToArray(loot, lootItem)) return createError(mapObj, "loot item in loot");
    }

    cJSON* enemy = cJSON_AddArrayToObject(roomObj, ENEMY);
    if (enemy == NULL) return createError(mapObj, ENEMY);
    if (room->enemy != NULL) {
      cJSON* enemyEntity = cJSON_CreateString(room->enemy);
      if (enemyEntity == NULL) return createError(mapObj, "enemy entity");
      if (!cJSON_AddItemToArray(enemy, enemyEntity)) return createError(mapObj, "enemy entity in enemy");
    }
  }

  char* mapState = cJSON_Print(mapObj);

  cJSON_Delete(mapObj);
  deleteTable(table);

  return mapState;
}

/**
 * Saves the current state of the maze.
 */
static void saveMap() {
  char* mapState = createMapState();
  if (mapState == NULL) handleError(ERR_DATA, ERROR, "Could not create map state!\n");

  char filename[35];
  sprintf(filename, "%s/maps/map_save.json", SAVE_DIR);

  FILE* file = fopen(filename, "w");
  if (file == NULL) handleError(ERR_IO, ERROR, "Unable to create the save!\n");
  else {
    int written = fprintf(file, "%s", mapState);
    if (written <= 0) handleError(ERR_IO, ERROR, "Unable to save the map data!\n");
  }

  fclose(file);

  cJSON_free(mapState);
}

/**
 * Creates the player state.
 * @return The player state as JSON string
 */
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

/**
 * Saves the current state of the player.
 */
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

/**
 * Loads the player data.
 * @return The player
 */
static SoulWorker* loadPlayer() {
  char filename[30];
  sprintf(filename, "%s/player_save.json", SAVE_DIR);


  cJSON* root = readData(filename);
  if (root == NULL) handleError(ERR_DATA, FATAL, "Could not parse JSON!\n");


  cJSON* name = cJSON_GetObjectItemCaseSensitive(root, NAME);
  if (name == NULL) handleError(ERR_DATA, FATAL, "No name data found!\n");

  cJSON* hp = cJSON_GetObjectItemCaseSensitive(root, HP);
  if (hp == NULL) handleError(ERR_DATA, FATAL, "No hp data found!\n");

  cJSON* maxHP = cJSON_GetObjectItemCaseSensitive(root, MAX_HP);
  if (maxHP == NULL) handleError(ERR_DATA, FATAL, "No maxHP data found!\n");

  cJSON* invCount = cJSON_GetObjectItemCaseSensitive(root, INV_COUNT);
  if (invCount == NULL) handleError(ERR_DATA, FATAL, "No invCount data found!\n");

  cJSON* xp = cJSON_GetObjectItemCaseSensitive(root, XP);
  if (xp == NULL) handleError(ERR_DATA, FATAL, "No xp data found!\n");

  cJSON* room = cJSON_GetObjectItemCaseSensitive(root, ROOM);
  if (room == NULL) handleError(ERR_DATA, FATAL, "No room data found!\n");
  cJSON* roomId = cJSON_GetObjectItemCaseSensitive(room, ID);
  if (roomId == NULL) handleError(ERR_DATA, FATAL, "No roomID data found!\n");
  cJSON* roomMap = cJSON_GetObjectItemCaseSensitive(room, MAP);
  if (roomMap == NULL) handleError(ERR_DATA, FATAL, "No roomMap data found!\n");

  cJSON* inv = cJSON_GetObjectItemCaseSensitive(root, INV);
  if (inv == NULL) handleError(ERR_DATA, FATAL, "No inventory data found!\n");


  int nameLen = strlen(name->valuestring);
  char* playerName = (char*) malloc(nameLen + 1);
  if (playerName == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for the player name!\n");
  strncpy(playerName, name->valuestring, nameLen+1);

  SoulWorker* player = initSoulWorker(playerName);
  player->hp = hp->valueint;
  player->xp = xp->valueint;
  player->invCount = invCount->valueint;
  player->maxHP = maxHP->valueint;

  player->room = findRoom(maze->entry, (char)roomId->valueint);
  if (player->room == NULL|| player->room == (void*)((long long)NO_EXIT)) handleError(ERR_DATA, FATAL, "Could not find room!\n");

  // Make sure room is the same
  if (player->room->id != (char)(roomId->valueint)) handleError(ERR_DATA, FATAL, "Room ID does not match!\n");


  for(int i = 0; i < cJSON_GetArraySize(inv); i++) {
    cJSON* invItem = cJSON_GetArrayItem(inv, i);
    cJSON* itemItem = cJSON_GetObjectItemCaseSensitive(invItem, ITEM);
    cJSON* itemCount = cJSON_GetObjectItemCaseSensitive(itemCount, COUNT);

    player->inv[i]._item = (item_t)itemItem->valuestring;
    player->inv[i].count = itemCount->valueint;
  }

  cJSON_Delete(root);

  return player;
}

/**
 * Loads the maze data. Just a wrapper to initMaze.
 * @return The maze
 */
static Maze* loadMap() {
  char filename[35];
  sprintf(filename, "%s/maps/map_save.json", SAVE_DIR);

  return initMaze(filename);
}

/**
 * Loads a saved game.
 */
void loadGame() {
  printf("Loading map...\n");
  maze = loadMap();
  printf("Map loaded!\n");

  printf("Loading player...\n");
  player = loadPlayer();
  printf("Player loaded!\n");

  printf("The game has been loaded!\n");
}