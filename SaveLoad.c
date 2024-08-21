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
#define GEAR "gear"

#define IS_ENTRY "isEntry"
#define INFO "info"
#define EXITS "exits"
#define LOOT "loot"
#define ENEMY "enemy"
#define HAS_BOSS "hasBoss"

const str SAVE_DIR = "./data/saves";

// itoa is in assembly (just for fun), change it to C later
/**
 * Converts integer n into a character string.
 * @param n The integer
 * @param buffer The buffer to hold the string
 * @return The integer as a string
 */
extern str itoa(int n, char* buffer);

/**
 * Handles errors regarding cJSON objects.
 * @param obj The cJSON object
 * @param type The JSON object tag
 * @return NULL
 */
static str createError(cJSON* obj, const str type) {
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
static Room* findRoom(Room* room, byte id, DArray* visited) {
  if (room != (void*) ((long long) NO_EXIT)) {
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

static bool saveSoulWeapon(cJSON* parentObj, SoulWeapon* data) {
  cJSON* name = cJSON_AddStringToObject(parentObj, NAME, data->name);
  if (name == NULL) { createError(parentObj, "SoulWeapon name"); return false; }

  cJSON* atk = cJSON_AddNumberToObject(parentObj, "atk", data->atk);
  if (atk == NULL) { createError(parentObj, "SoulWeapon atk"); return false; }

  cJSON* acc = cJSON_AddNumberToObject(parentObj, "acc", data->acc);
  if (acc == NULL) { createError(parentObj, "SoulWeapon acc"); return false; }

  cJSON* atkCrit = cJSON_AddNumberToObject(parentObj, "atk_crit", data->atk_crit);
  if (atkCrit == NULL) { createError(parentObj, "SoulWeapon atk crit"); return false; }

  cJSON* critDmg = cJSON_AddNumberToObject(parentObj, "atk_crit_dmg", data->atk_crit_dmg);
  if (critDmg == NULL) { createError(parentObj, "SoulWeapon atk crit dmg"); return false; }

  cJSON* lvl = cJSON_AddNumberToObject(parentObj, LVL, data->lvl);
  if (lvl == NULL) { createError(parentObj, "SoulWeapon lvl"); return false; }

  cJSON* upgrades = cJSON_AddNumberToObject(parentObj, "upgrades", data->upgrades);
  if (upgrades == NULL) { createError(parentObj, "SoulWeapon upgrades"); return false; }

  cJSON* durability = cJSON_AddNumberToObject(parentObj, "durability", data->durability);
  if (durability == NULL) { createError(parentObj, "SoulWeapon durability"); return false; }

  return true;
}

static bool saveArmor(cJSON* parentObj, Armor* data) {
  cJSON* name = cJSON_AddStringToObject(parentObj, NAME, data->name);
  if (name == NULL) { createError(parentObj, "armor name"); return false; }

  cJSON* type = cJSON_AddNumberToObject(parentObj, TYPE, data->type);
  if (type == NULL) { createError(parentObj, "armor type"); return false; }

  cJSON* acc = cJSON_AddNumberToObject(parentObj, "acc", data->acc);
  if (acc == NULL) { createError(parentObj, "armor acc"); return false; }

  cJSON* def = cJSON_AddNumberToObject(parentObj, "def", data->def);
  if (def == NULL) { createError(parentObj, "armor def"); return false; }
  
  cJSON* lvl = cJSON_AddNumberToObject(parentObj, LVL, data->lvl);
  if (lvl == NULL) { createError(parentObj, "armor lvl"); return false; }

  return true;
}

static bool saveGear(cJSON* parentObj, Gear data) {
  cJSON* _null = (void*) ((long long) 0xFEEDBEAD); // For when a gear piece does not exist to be saved

  cJSON* gear = cJSON_AddObjectToObject(parentObj, "gear");
  if (gear == NULL) { createError(parentObj, GEAR); return false; }

  if (data.sw == NULL) _null = cJSON_AddNullToObject(gear, "sw");
  else {
    cJSON* sw = cJSON_AddObjectToObject(gear, "sw");
    if (sw == NULL) { createError(parentObj, "SoulWeapon"); return false; }

    bool _sw = saveSoulWeapon(sw, data.sw);
    if (!_sw) { createError(parentObj, "SoulWeapon"); return false; }
  }

  if (data.helmet == NULL) _null = cJSON_AddNullToObject(gear, "helmet");
  else {
    cJSON* helmet = cJSON_AddObjectToObject(gear, "helmet");
    if (helmet == NULL) { createError(parentObj, "helmet"); return false; }

    bool _helmet = saveArmor(helmet, data.helmet);
    if (!_helmet) { createError(parentObj, "helmet"); return false; }
  }

  if (data.guard == NULL) _null = cJSON_AddNullToObject(gear, "guard");
  else {
    cJSON* guard = cJSON_AddObjectToObject(gear, "guard");
    if (guard == NULL) { createError(parentObj, "guard"); return false; }

    bool _guard = saveArmor(guard, data.guard);
    if (!_guard) { createError(parentObj, "guard"); return false; }
  }

  if (data.chestplate == NULL) _null = cJSON_AddNullToObject(gear, "chestplate");
  else {
    cJSON* chestplate = cJSON_AddObjectToObject(gear, "chestplate");
    if (chestplate == NULL) { createError(parentObj, "chestplate"); return false; }

    bool _chestplate = saveArmor(chestplate, data.chestplate);
    if (!_chestplate) { createError(parentObj, "chestplate"); return false; }
  }

  if (data.boots == NULL) _null = cJSON_AddNullToObject(gear, "boots");
  else {
    cJSON* boots = cJSON_AddObjectToObject(gear, "boots");
    if (boots == NULL) { createError(parentObj, "boots"); return false; }

    bool _boots = saveArmor(boots, data.boots);
    if (!_boots) { createError(parentObj, "boots"); return false; }
  }

  // It would be better to check if AddNullToObject is checked for nullity
  //  right after it is created in order to avoid processing for the other data
  //  but I am not going to add more if statements because I'm too lazy and will not like how it will look
  if (_null == NULL) { createError(parentObj, "null"); return false; }

  return true;
}

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
      break;
    case SLIME_T:
      Slime* slime = (Slime*) lootItem;

      cJSON* slimeDesc = cJSON_AddStringToObject(_lootItem, "description", slime->desc);
      if (slimeDesc == NULL) { createError(parentObj, "item slime description"); return NULL; }
      break;
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

static bool saveStats(cJSON* parentObj, Stats* data) {
  cJSON* stats = cJSON_CreateObject();
  if (stats == NULL) { createError(parentObj, "stats"); return false; }

  cJSON* atkArr = cJSON_AddArrayToObject(stats, "ATK");
  if (atkArr == NULL) { createError(parentObj, "atk container"); return false; }
  cJSON* atk = cJSON_CreateNumber(data->ATK);
  if (atk == NULL) { createError(parentObj, "atk value"); return false; }
  if (!cJSON_AddItemToArray(atkArr, atk)) { createError(parentObj, "atk"); return false; }

  cJSON* defArr = cJSON_AddArrayToObject(stats, "DEF");
  if (defArr == NULL) { createError(parentObj, "def container"); return false; }
  cJSON* def = cJSON_CreateNumber(data->DEF);
  if (def == NULL) { createError(parentObj, "def value"); return false; }
  if (!cJSON_AddItemToArray(defArr, def)) { createError(parentObj, "def"); return false; }

  cJSON* accArr = cJSON_AddArrayToObject(stats, "ACC");
  if (accArr == NULL) { createError(parentObj, "acc container"); return false; }
  cJSON* acc = cJSON_CreateNumber(data->ACC);
  if (acc == NULL) { createError(parentObj, "acc value"); return false; }
  if (!cJSON_AddItemToArray(accArr, acc)) { createError(parentObj, "acc"); return false; }

  cJSON* critDmgArr = cJSON_AddArrayToObject(stats, "ATK_CRIT_DMG");
  if (critDmgArr == NULL) { createError(parentObj, "atk crit dmg container"); return false; }
  cJSON* critDmg = cJSON_CreateNumber(data->ATK_CRIT_DMG);
  if (critDmg == NULL) { createError(parentObj, "atk crit dmg value"); return false; }
  if (!cJSON_AddItemToArray(critDmgArr, critDmg)) { createError(parentObj, "atk crit dmg"); return false; }

  cJSON* atkCritArr = cJSON_AddArrayToObject(stats, "ATK_CRIT");
  if (atkCritArr == NULL) { createError(parentObj, "atk crit container"); return false; }
  cJSON* atkCrit = cJSON_CreateNumber(data->ATK_CRIT);
  if (atkCrit == NULL) { createError(parentObj, "atk crit value"); return false; }
  if (!cJSON_AddItemToArray(atkCritArr, atkCrit)) { createError(parentObj, "atk crit"); return false; }

  return cJSON_AddItemToObject(parentObj, "stats", stats);
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

  cJSON* hpArr = cJSON_AddArrayToObject(enemy, HP);
  if (hpArr == NULL) { createError(enemy, "enemy hp container"); return NULL; }
  cJSON* hp = cJSON_CreateNumber(_enemy->enemy->hp);
  if (hp == NULL) { createError(enemy, "enemy hp value"); return NULL; }
  if (!cJSON_AddItemToArray(hpArr, hp)) { createError(enemy, "enemy hp"); return NULL; }

  cJSON* lvl = cJSON_AddNumberToObject(enemy, LVL, _enemy->enemy->lvl);
  if (lvl == NULL) { createError(enemy, "enemy lvl"); return NULL; }

  bool stats = saveStats(enemy, _enemy->enemy->stats);
  if (!stats) { createError(enemy, "enemy stats"); return NULL; }

  if (hasBoss) {
    bool gear = saveGear(enemy, _enemy->boss->gearDrop);
    if (!gear) { createError(enemy, "boss gear drop"); return NULL; }
  }

  return enemy;
}

/**
 * Creates the maze state for saving.
 * @return The maze as a JSON string
 */
static str createMapState() {
  Table* table = initTable();
  if (table == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for the table!\n");

  Room* room = maze->entry;
  addAndRecurse(room, table);
  room = NULL;

  if (table->len != maze->size) { handleError(ERR_DATA, WARNING, "Table size does not equal maze size!\n"); return NULL; }

  // The table SHOULD have all of the rooms
  // Time to create the JSON object
  cJSON* mapObj = cJSON_CreateObject();
  if (mapObj == NULL) return createError(mapObj, "map");
  for (int i = 0; i < table->len; i++) {
    room = table->rooms[i];
    // Creating each room

    char buffer[2];
    str idAsChar = itoa(room->id, buffer);

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

  str mapState = cJSON_Print(mapObj);

  cJSON_Delete(mapObj);
  deleteTable(table);

  return mapState;
}

/**
 * Saves the current state of the maze.
 * @return True if the map was saved, false otherwise
 */
static bool saveMap() {
  str mapState = createMapState();
  if (mapState == NULL) handleError(ERR_DATA, WARNING, "Could not create map state!\n");

  if (mapState != NULL) {
    char filename[35];
    sprintf(filename, "%s/maps/map_save.json", SAVE_DIR);

    FILE* file = fopen(filename, "w");
    if (file == NULL) { handleError(ERR_IO, WARNING, "Unable to create the save!\n"); return false; }
    else {
      int written = fprintf(file, "%s", mapState);
      if (written <= 0) { handleError(ERR_IO, WARNING, "Unable to save the map data!\n"); return false; }
    }

    fclose(file);

    cJSON_free(mapState);

    return true;
  }

  return false;
}

/**
 * Creates the player state.
 * @return The player state as JSON string
 */
static str createPlayerState() {
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

      cJSON* itemItem = saveLootItem(item, player->inv[i]._item, player->inv[i].type);
      if (itemItem == NULL) return createError(playerObj, "item item");

      cJSON* itemType = cJSON_AddNumberToObject(item, TYPE, player->inv[i].type);
      if (itemType == NULL) return createError(playerObj, TYPE);

      cJSON* itemCount = cJSON_AddNumberToObject(item, COUNT, player->inv[i].count);
      if (itemCount == NULL) return createError(playerObj, COUNT);

      if(!cJSON_AddItemToArray(playerInv, item)) return createError(playerObj, "item in inventory");
    }
  }

  bool gear = saveGear(playerObj, player->gear);
  if (!gear) return createError(playerObj, GEAR);

  bool stats = saveStats(playerObj, player->stats);
  if (!stats) { createError(playerObj, "player stats"); return NULL; }

  str playerState = cJSON_Print(playerObj);

  cJSON_Delete(playerObj);
  return playerState;
}

/**
 * Saves the current state of the player.
 * @return True if the player was saved, false otherwise
 */
static bool savePlayer() {
  str playerState = createPlayerState();
  if (playerState == NULL) handleError(ERR_DATA, WARNING, "Could not create player state!\n");

  if (playerState != NULL) {
    char filename[32];
    sprintf(filename, "%s/player_save.json", SAVE_DIR);

    FILE* file = fopen(filename, "w");
    if (file == NULL) { handleError(ERR_IO, WARNING, "Unable to create the save!\n"); return false; } 
    else {
      int written = fprintf(file, "%s", playerState);

      if (written <= 0) { handleError(ERR_IO, WARNING, "Unable to save the player data!\n"); return false; }
    }

    fclose(file);

    cJSON_free(playerState);

    return true;
  }

  return false;
}

void saveGame() {
  if (savePlayer() && saveMap()) printf("The game has been saved!\n");
  else printf("The game could not be saved!\n");
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

  cJSON* dzenai = cJSON_GetObjectItemCaseSensitive(root, DZ);
  if (dzenai == NULL) handleError(ERR_DATA, FATAL, "No dzenai data found!\n");

  cJSON* lvl = cJSON_GetObjectItemCaseSensitive(root, LVL);
  if (lvl == NULL) handleError(ERR_DATA, FATAL, "No lvl data found!\n");

  cJSON* room = cJSON_GetObjectItemCaseSensitive(root, ROOM);
  if (room == NULL) handleError(ERR_DATA, FATAL, "No room data found!\n");
  cJSON* roomId = cJSON_GetObjectItemCaseSensitive(room, ID);
  if (roomId == NULL) handleError(ERR_DATA, FATAL, "No roomID data found!\n");
  cJSON* roomMap = cJSON_GetObjectItemCaseSensitive(room, MAP);
  if (roomMap == NULL) handleError(ERR_DATA, FATAL, "No roomMap data found!\n");

  cJSON* inv = cJSON_GetObjectItemCaseSensitive(root, INV);
  if (inv == NULL) handleError(ERR_DATA, FATAL, "No inventory data found!\n");


  int nameLen = strlen(name->valuestring);
  str playerName = (str) malloc(nameLen + 1);
  if (playerName == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for the player name!\n");
  strncpy(playerName, name->valuestring, nameLen+1);

  SoulWorker* player = initSoulWorker(playerName);
  player->hp = hp->valueint;
  player->xp = xp->valueint;
  player->invCount = invCount->valueint;
  player->maxHP = maxHP->valueint;
  player->dzenai = dzenai->valueint;
  player->lvl = lvl->valueint;

  DArray* visited = initDArray(maze->size);

  player->room = findRoom(maze->entry, (char)roomId->valueint, visited);

  dArrayFree(visited);

  if (player->room == NULL|| player->room == (void*)((long long)NO_EXIT)) handleError(ERR_DATA, FATAL, "Could not find room!\n");

  // Make sure room is the same
  if (player->room->id != (byte)(roomId->valueint)) handleError(ERR_DATA, FATAL, "Room ID does not match!\n");


  for (int i = 0; i < cJSON_GetArraySize(inv); i++) {
    cJSON* invItem = cJSON_GetArrayItem(inv, i);

    cJSON* itemType = cJSON_GetObjectItemCaseSensitive(invItem, TYPE);

    Item* item = createItem(invItem, itemType->valueint);
    player->inv[i]._item = item->_item;
    player->inv[i].type = item->type;
    player->inv[i].count = item->count;
    free(item);
    item = NULL;
  }

  // TODO: Make loadGear and loadStats, use with loading enemies???

  cJSON* stats = cJSON_GetObjectItemCaseSensitive(root, "stats");
  if (stats == NULL) handleError(ERR_DATA, FATAL, "No stats data found!\n");

  cJSON* atk = cJSON_GetObjectItemCaseSensitive(stats, "ATK");
  if (atk == NULL) handleError(ERR_DATA, FATAL, "No atk stats data found!\n");
  player->stats->ATK = (cJSON_GetArrayItem(atk, 0))->valueint;

  cJSON* def = cJSON_GetObjectItemCaseSensitive(stats, "DEF");
  if (def == NULL) handleError(ERR_DATA, FATAL, "No def stats data found!\n");
  player->stats->DEF = (cJSON_GetArrayItem(def, 0))->valueint;

  cJSON* acc = cJSON_GetObjectItemCaseSensitive(stats, "ACC");
  if (acc == NULL) handleError(ERR_DATA, FATAL, "No acc stats data found!\n");
  player->stats->ACC = (cJSON_GetArrayItem(acc, 0))->valueint;

  cJSON* atkCrit = cJSON_GetObjectItemCaseSensitive(stats, "ATK_CRIT");
  if (atkCrit == NULL) handleError(ERR_DATA, FATAL, "No atk_crit stats data found!\n");
  player->stats->ATK_CRIT = (cJSON_GetArrayItem(atkCrit, 0))->valuedouble;

  cJSON* critDmg = cJSON_GetObjectItemCaseSensitive(stats, "ATK_CRIT_DMG");
  if (critDmg == NULL) handleError(ERR_DATA, FATAL, "No atk_crit_dmg stats data found!\n");
  player->stats->ATK_CRIT_DMG = (cJSON_GetArrayItem(critDmg, 0))->valueint;


  cJSON* gear = cJSON_GetObjectItemCaseSensitive(root, "gear");
  if (gear == NULL) handleError(ERR_DATA, FATAL, "No gear data found!\n");

  cJSON* sw = cJSON_GetObjectItemCaseSensitive(gear, "sw");
  if (sw == NULL) handleError(ERR_DATA, FATAL, "No SoulWeapon data found!\n");
  if (!cJSON_IsNull(sw)) player->gear.sw = createSoulWeapon(sw);

  cJSON* helmet = cJSON_GetObjectItemCaseSensitive(gear, "helmet");
  if (helmet == NULL) handleError(ERR_DATA, FATAL, "No helmet data found!\n");
  if (!cJSON_IsNull(helmet)) player->gear.helmet = createArmor(helmet);

  cJSON* guard = cJSON_GetObjectItemCaseSensitive(gear, "guard");
  if (guard == NULL) handleError(ERR_DATA, FATAL, "No  data found!\n");
  if (!cJSON_IsNull(guard)) player->gear.guard = createArmor(guard);

  cJSON* chestplate = cJSON_GetObjectItemCaseSensitive(gear, "chestplate");
  if (chestplate == NULL) handleError(ERR_DATA, FATAL, "No  data found!\n");
  if (!cJSON_IsNull(chestplate)) player->gear.chestplate = createArmor(chestplate);

  cJSON* boots = cJSON_GetObjectItemCaseSensitive(gear, "boots");
  if (boots == NULL) handleError(ERR_DATA, FATAL, "No  data found!\n");
  if (!cJSON_IsNull(boots)) player->gear.boots = createArmor(boots);


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