#include <time.h>
#include <stdio.h>

#include "SaveLoad.h"
#include "cJSON.h"
#include "Error.h"


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


static char* createError(cJSON* obj, const char* type) {
  cJSON_Delete(obj);

  handleError(ERR_DATA, ERROR, "Could not create JSON for %s!\n", type);

  return NULL;
}


static void saveMap() {

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
  for(int i = 0; i < INV_CAP; i++) {
    if (player->inv[i]._item != NO_ITEM) {
      cJSON* item = cJSON_CreateObject();
      if (item == NULL) return createError(playerObj, ITEM);

      cJSON* itemItem = cJSON_AddStringToObject(item, ITEM, (char*) player->inv[i]._item);
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
  char* dir = "./data/saves";

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
  // saveMap();

  printf("The game has been saved!\n");
}

void loadGame(const char* savefile) {

}
