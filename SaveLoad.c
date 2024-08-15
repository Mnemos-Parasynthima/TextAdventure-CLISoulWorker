#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SaveLoad.h"
#include "LoadJSON.h"
#include "Error.h"
#include "Setup.h"
#include "DArray.h"


#define NO_ITEM 0x0
#define NAME "name"
#define HP "hp"
#define MAX_HP "maxHP"
#define INV_COUNT "invCount"
#define XP "xp"
#define LVL "lvl"
#define DZ "dzenai"
#define ROOM "room"
#define INV "inventory"
#define ID "id"
#define MAP "map"
#define ITEM "item"
#define TYPE "type"
#define COUNT "count"

#define IS_ENTRY "isEntry"
#define INFO "info"
#define EXITS "exits"
#define LOOT "loot"
#define ENEMY "enemy"
#define HAS_BOSS "hasBoss"

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

  handleError(ERR_DATA, WARNING, "Could not create JSON for %s!\n", type);

  return NULL;
}

/**
 * Finds a room with the given ID.
 * @param room The room to look at
 * @param id The target ID
 * @return Room with matching ID
 */
static Room* findRoom(Room* room, char id, DArray* visited) {
  if (room != (void*)((long long)NO_EXIT)) {
    if (room->id == id) return room;

    if (dArrayExists(visited, room->id)) return NULL;

    dArrayAdd(visited, room->id);

    for (int i = 0; i < 4; i++) {
      Room* target = findRoom(room->exits[i], id, visited);
      if (target != NULL) return target;
    }
  }

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

static void saveSoulWeapon(cJSON* parentObj, SoulWeapon* data) {
  cJSON* name = cJSON_AddStringToObject(parentObj, NAME, data->name);
  if (name == NULL) { createError(parentObj, "SoulWeapon name"); return; }

  cJSON* atk = cJSON_AddNumberToObject(parentObj, "atk", data->atk);
  if (atk == NULL) { createError(parentObj, "SoulWeapon atk"); return; }

  cJSON* acc = cJSON_AddNumberToObject(parentObj, "acc", data->acc);
  if (acc == NULL) { createError(parentObj, "SoulWeapon acc"); return; }

  cJSON* atkCrit = cJSON_AddNumberToObject(parentObj, "atk_crit", data->atk_crit);
  if (atkCrit == NULL) { createError(parentObj, "SoulWeapon atk crit"); return; }

  cJSON* critDmg = cJSON_AddNumberToObject(parentObj, "atk_crit_dmg", data->atk_crit_dmg);
  if (critDmg == NULL) { createError(parentObj, "SoulWeapon atk crit dmg"); return; }

  cJSON* lvl = cJSON_AddNumberToObject(parentObj, LVL, data->lvl);
  if (lvl == NULL) { createError(parentObj, "SoulWeapon lvl"); return; }

  cJSON* upgrades = cJSON_AddNumberToObject(parentObj, "upgrades", data->upgrades);
  if (upgrades == NULL) { createError(parentObj, "SoulWeapon upgrades"); return; }

  cJSON* durability = cJSON_AddNumberToObject(parentObj, "durability", data->durability);
  if (durability == NULL) { createError(parentObj, "SoulWeapon durability"); return; }
}

static void saveArmor(cJSON* parentObj, Armor* data) {
  cJSON* name = cJSON_AddStringToObject(parentObj, NAME, data->name);
  if (name == NULL) { createError(parentObj, "armor name"); return; }

  cJSON* type = cJSON_AddNumberToObject(parentObj, TYPE, data->type);
  if (type == NULL) { createError(parentObj, "armor type"); return; }

  cJSON* acc = cJSON_AddNumberToObject(parentObj, "acc", data->acc);
  if (acc == NULL) { createError(parentObj, "armor acc"); return; }

  cJSON* def = cJSON_AddNumberToObject(parentObj, "def", data->def);
  if (def == NULL) { createError(parentObj, "armor def"); return; }
  
  cJSON* lvl = cJSON_AddNumberToObject(parentObj, LVL, data->lvl);
  if (lvl == NULL) { createError(parentObj, "armor lvl"); return; }
}

static cJSON* saveGear(cJSON* parentObj, Gear data) { return NULL; }

static cJSON* saveLootItem(cJSON* parentObj, void* lootItem, item_t lootType) {
  cJSON* _lootItem = cJSON_AddObjectToObject(parentObj, ITEM);
  if (_lootItem == NULL) { createError(parentObj, "loot item"); return NULL; }

  switch (lootType) {
    case SOULWEAPON_T:
      saveSoulWeapon(_lootItem, (SoulWeapon*) lootItem);
      break;
    case HELMET_T:
    case SHOULDER_GUARD_T:
    case CHESTPLATE_T:
    case BOOTS_T:
      saveArmor(_lootItem, (Armor*) lootItem);
      break;
    case HP_KITS_T:
      HPKit* hpKit = (HPKit*) lootItem;

      cJSON* hpKitDesc = cJSON_AddStringToObject(_lootItem, "description", hpKit->desc);
      if (hpKitDesc == NULL) { createError(parentObj, "item HP Kit description"); return NULL; }

      cJSON* hpKitT = cJSON_AddNumberToObject(_lootItem, TYPE, hpKit->type);
      if (hpKitT == NULL) { createError(parentObj, "item HP Kit type"); return NULL; }
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
    case ARMOR_UPGRADE_MATERIALS_T:
      Upgrade* upgrade = (Upgrade*) lootItem;

      cJSON* upgradeDesc = cJSON_AddStringToObject(_lootItem, "description", upgrade->desc);
      if (upgradeDesc == NULL) { createError(parentObj, "item upgrade material description"); return NULL; }

      cJSON* upgradeT = cJSON_AddNumberToObject(_lootItem, TYPE, upgrade->type);
      if (upgradeT == NULL) { createError(parentObj, "item upgrade material type"); return NULL; }

      cJSON* upgradeR = cJSON_AddNumberToObject(_lootItem, "rank", upgrade->rank);
      if (upgradeR == NULL) { createError(parentObj, "item upgrade material rank"); return NULL; }
    case SLIME_T:
      Slime* slime = (Slime*) lootItem;

      cJSON* slimeDesc = cJSON_AddStringToObject(_lootItem, "description", slime->desc);
      if (slimeDesc == NULL) { createError(parentObj, "item slime description"); return NULL; }
    default:
      break;
  }

  return _lootItem;
}

static cJSON* saveLoot(Item* loot) {
  cJSON* _loot = cJSON_CreateObject();
  if (_loot == NULL) { createError(_loot, "loot"); return NULL; }

  cJSON* item = saveLootItem(_loot, loot->_item, loot->type);
  if (item == NULL) { createError(_loot, "loot item"); return NULL; }

  cJSON* type = cJSON_AddNumberToObject(_loot, TYPE, loot->type);
  if (type == NULL) { createError(_loot, "loot type"); return NULL; }

  cJSON* count = cJSON_AddNumberToObject(_loot, "count", loot->count);
  if (count == NULL) { createError(_loot, "loot count"); return NULL; }

  return _loot;
}

static cJSON* saveStats(cJSON* parentObj, Stats* data) {
  cJSON* stats = cJSON_CreateObject();
  if (stats == NULL) { createError(parentObj, "stats"); return NULL; }

  cJSON* atk = cJSON_AddNumberToObject(stats, "ATK", data->ATK);
  if (atk == NULL) { createError(parentObj, "atk"); return NULL; }

  cJSON* def = cJSON_AddNumberToObject(stats, "DEF", data->DEF);
  if (def == NULL) { createError(parentObj, "def"); return NULL; }

  cJSON* acc = cJSON_AddNumberToObject(stats, "ACC", data->ACC);
  if (acc == NULL) { createError(parentObj, "acc"); return NULL; }

  cJSON* critDmg = cJSON_AddNumberToObject(stats, "ATK_CRIT_DMG", data->ATK_CRIT_DMG);
  if (critDmg == NULL) { createError(parentObj, "critDmg"); return NULL; }

  cJSON* atkCrit = cJSON_AddNumberToObject(stats, "ATK_CRIT", data->ATK_CRIT);
  if (atkCrit == NULL) { createError(parentObj, "atkCrit"); return NULL; }

  return stats;
}


static cJSON* saveEnemy(EnemyU* _enemy, bool hasBoss) {
  /**
   * NOTE
   * enemy union contains an address, either to Enemy or Boss,
   * but an address nonetheless. Thus no matter which type,
   * it'll still lead to the object.
   * Considering Boss has the Enemy structure at the beginning,
   * its data will be in the same relative order as a regular Enemy
   * Thus, doing _enemy->enemy->[] will do the same pointer arithmetic
   * as _enemy->boss->base->[].
   * However, since Boss has 40B of data more than Enemy,
   * that will need to be saved using the lens as Boss.
   */


  cJSON* enemy = cJSON_CreateObject();
  if (enemy == NULL) { createError(enemy, "enemy"); return NULL; }

  cJSON* name = cJSON_AddStringToObject(enemy, NAME, _enemy->enemy->name);
  if (name == NULL) { createError(enemy, "enemy name"); return NULL; }

  cJSON* xpPoints = cJSON_AddNumberToObject(enemy, "xpPoints", _enemy->enemy->xpPoints);
  if (xpPoints == NULL) { createError(enemy, "enemy xpPoints"); return NULL; }

  cJSON* hp = cJSON_AddNumberToObject(enemy, HP, _enemy->enemy->hp);
  if (hp == NULL) { createError(enemy, "enemy hp"); return NULL; }

  cJSON* lvl = cJSON_AddNumberToObject(enemy, LVL, _enemy->enemy->lvl);
  if (lvl == NULL) { createError(enemy, "enemy lvl"); return NULL; }

  cJSON* stats = saveStats(enemy, _enemy->enemy->stats);
  if (stats == NULL) { createError(enemy, "enemy stats"); return NULL; }

  if (hasBoss) {
    cJSON* gear = saveGear(enemy, _enemy->boss->gearDrop);
    if (gear == NULL) { createError(enemy, "boss gear drop"); return NULL; }
  }

  return enemy;
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

    cJSON* hasBoss = cJSON_AddNumberToObject(roomObj, HAS_BOSS, (room->hasBoss) ? 1 : 0);
    if (hasBoss == NULL) return createError(mapObj, HAS_BOSS);

    cJSON* exits = cJSON_AddArrayToObject(roomObj, EXITS);
    if (exits == NULL) return createError(mapObj, EXITS);
    for (int i = 0; i < 4; i++) {
      Room* roomExit = room->exits[i];

      cJSON* exit = cJSON_CreateNumber((roomExit == (void*)((long long)NO_EXIT) ? -1 : roomExit->id));
      if (exit == NULL) return createError(mapObj, "exit");

      if (!cJSON_AddItemToArray(exits, exit)) return createError(mapObj, "exit in exits");
    }

    // Create the arrays just to be in compliance to format
    // But no need to add when none are present

    cJSON* loot = cJSON_AddArrayToObject(roomObj, LOOT);
    if (loot == NULL) return createError(mapObj, LOOT);
    if (room->loot != NULL) {
      cJSON* _loot = saveLoot(room->loot);
      if (_loot == NULL) return createError(mapObj, "item loot");
      if (!cJSON_AddItemToArray(loot, _loot)) return createError(mapObj, "item loot in loot");
    }

    cJSON* enemy = cJSON_AddArrayToObject(roomObj, ENEMY);
    if (enemy == NULL) return createError(mapObj, ENEMY);
    if (room->enemy.enemy != NULL) {
      cJSON* enemyEntity = saveEnemy(&(room->enemy), room->hasBoss);
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
  if (mapState == NULL) handleError(ERR_DATA, WARNING, "Could not create map state!\n");

  char filename[35];
  sprintf(filename, "%s/maps/map_save.json", SAVE_DIR);

  FILE* file = fopen(filename, "w");
  if (file == NULL) handleError(ERR_IO, WARNING, "Unable to create the save!\n");
  else {
    int written = fprintf(file, "%s", mapState);
    if (written <= 0) handleError(ERR_IO, WARNING, "Unable to save the map data!\n");
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

  cJSON* playerXP = cJSON_AddNumberToObject(playerObj, XP, player->xp);
  if (playerXP == NULL) return createError(playerObj, XP);

  cJSON* playerLvl = cJSON_AddNumberToObject(playerObj, LVL, player->lvl);
  if (playerLvl == NULL) return createError(playerObj, LVL);

  cJSON* playerHP = cJSON_AddNumberToObject(playerObj, HP, player->hp);
  if (playerHP == NULL) return createError(playerObj, HP);

  cJSON* playerMaxHP = cJSON_AddNumberToObject(playerObj, MAX_HP, player->maxHP);
  if (playerMaxHP == NULL) return createError(playerObj, MAX_HP);

  cJSON* playerDZ = cJSON_AddNumberToObject(playerObj, DZ, player->dzenai);
  if (playerDZ == NULL) return createError(playerObj, DZ);

  cJSON* playerInvCount = cJSON_AddNumberToObject(playerObj, INV_COUNT, player->invCount);
  if (playerInvCount == NULL) return createError(playerObj, INV_COUNT);

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

      cJSON* itemItem = saveLootItem(itemItem, player->inv[i]._item, player->inv[i].type);
      if (itemItem == NULL) return createError(playerObj, "item item");

      cJSON* itemType = cJSON_AddNumberToObject(item, TYPE, player->inv[i].type);
      if (itemType == NULL) return createError(playerObj, TYPE);

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
  if (playerState == NULL) handleError(ERR_DATA, WARNING, "Could not create player state!\n");

  char filename[32];
  sprintf(filename, "%s/player_save.json", dir);

  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    handleError(ERR_IO, WARNING, "Unable to create the save!\n");
  } else {
    int written = fprintf(file, "%s", playerState);

    if (written <= 0) handleError(ERR_IO, WARNING, "Unable to save the player data!\n");
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


  DArray* visited = initDArray(maze->size);

  player->room = findRoom(maze->entry, (char)roomId->valueint, visited);

  dArrayFree(visited);

  if (player->room == NULL|| player->room == (void*)((long long)NO_EXIT)) handleError(ERR_DATA, FATAL, "Could not find room!\n");

  // Make sure room is the same
  if (player->room->id != (char)(roomId->valueint)) handleError(ERR_DATA, FATAL, "Room ID does not match!\n");


  for (int i = 0; i < cJSON_GetArraySize(inv); i++) {
    cJSON* invItem = cJSON_GetArrayItem(inv, i);

    cJSON* itemItem = cJSON_GetObjectItemCaseSensitive(invItem, ITEM);
    cJSON* itemType = cJSON_GetObjectItemCaseSensitive(invItem, TYPE);
    cJSON* itemCount = cJSON_GetObjectItemCaseSensitive(invItem, COUNT);

    player->inv[i]._item = createItem(itemItem, itemType->valueint);
    player->inv[i].type = (item_t) itemType->valueint;
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