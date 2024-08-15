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

  cJSON* hasBoss = cJSON_GetObjectItemCaseSensitive(room, "hasBoss");
  if (hasBoss == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "hasBoss");

  cJSON* exits = cJSON_GetObjectItemCaseSensitive(room, "exits");
  if (exits == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "exits");
  if (cJSON_GetArraySize(exits) != 4) handleError(ERR_DATA, FATAL, "Room %d: Exits must only be 4!\n", roomId);
  cJSON* e = NULL;
  cJSON_ArrayForEach(e, exits) {
    if (e->valueint < -1) handleError(ERR_DATA, FATAL, "Room %d: exit markers cannot be less than -1!\n", roomId);
  };

  cJSON* loot = cJSON_GetObjectItemCaseSensitive(room, "loot");
  if (loot == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "loot");
  e = NULL;
  cJSON_ArrayForEach(e, loot) {
    cJSON* item = cJSON_GetObjectItemCaseSensitive(e, "item");
    if (item == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "loot item");

    cJSON* type = cJSON_GetObjectItemCaseSensitive(e, "type");
    if (type == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "loot type");

    cJSON* count = cJSON_GetObjectItemCaseSensitive(e, "count");
    if (count == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "loot count");
  }


  // TODO: When a system of all possible loot items is implemented
  //    add a check to make sure all the items in the loot table are valid
  //  Do the same for enemies.

  cJSON* enemy = cJSON_GetObjectItemCaseSensitive(room, "enemy");
  if (enemy == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy");
  e = NULL;
  cJSON_ArrayForEach(e, enemy) {
    cJSON* name = cJSON_GetObjectItemCaseSensitive(e, "name");
    if (name == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy name");

    cJSON* xpPoints = cJSON_GetObjectItemCaseSensitive(e, "xpPoints");
    if (xpPoints == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy xpPoints");

    cJSON* hp = cJSON_GetObjectItemCaseSensitive(e, "hp");
    if (hp == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy hp");

    cJSON* lvl = cJSON_GetObjectItemCaseSensitive(e, "lvl");
    if (lvl == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy lvl");

    cJSON* stats = cJSON_GetObjectItemCaseSensitive(e, "stats");
    if (stats == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy stats");
    cJSON* atk = cJSON_GetObjectItemCaseSensitive(stats, "ATK");
    if (atk == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy stats atk");
    cJSON* def = cJSON_GetObjectItemCaseSensitive(stats, "DEF");
    if (def == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy stats def");
    cJSON* acc = cJSON_GetObjectItemCaseSensitive(stats, "ACC");
    if (acc == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy stats acc");
    cJSON* critD = cJSON_GetObjectItemCaseSensitive(stats, "ATK_CRIT_DMG");
    if (critD == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy stats crit_dmg");
    cJSON* crit = cJSON_GetObjectItemCaseSensitive(stats, "ATK_CRIT");
    if (crit == NULL) handleError(ERR_DATA, FATAL, dataErr, roomId, "enemy stats crit");

    if (hasBoss->valueint == 1) {
      handleError(ERR_DATA, FATAL, "BOSS GEAR CHECK NOT IMPLEMENTED!\n");
    }
  }
}


static SoulWeapon* createSoulWeapon(cJSON* obj) {
  const char* errMsg = "Could not find data for SoulWeapon %s!\n";

  SoulWeapon* sw = (SoulWeapon*) malloc(sizeof(SoulWeapon));
  if (sw == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for SoulWeapon!\n");

  cJSON* name = cJSON_GetObjectItemCaseSensitive(obj, "name");
  if (name == NULL) handleError(ERR_DATA, FATAL, errMsg, "name");
  sw->name = (char*) malloc(strlen(name->valuestring) + 1);
  if (sw->name == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for SoulWeapon name!\n");
  strcpy(sw->name, name->valuestring);

  cJSON* atk = cJSON_GetObjectItemCaseSensitive(obj, "atk");
  if (atk == NULL) handleError(ERR_DATA, FATAL, errMsg, "atk");
  sw->atk = atk->valueint;

  cJSON* acc = cJSON_GetObjectItemCaseSensitive(obj, "acc");
  if (acc == NULL) handleError(ERR_DATA, FATAL, errMsg, "acc");
  sw->acc = acc->valueint;

  cJSON* atkCrit = cJSON_GetObjectItemCaseSensitive(obj, "atk_crit");
  if (atkCrit == NULL) handleError(ERR_DATA, FATAL, errMsg, "atk crit");
  sw->atk_crit = atkCrit->valuedouble;

  cJSON* critDmg = cJSON_GetObjectItemCaseSensitive(obj, "akt_crit_dmg");
  if (critDmg == NULL) handleError(ERR_DATA, FATAL, errMsg, "atk crit dmg");
  sw->atk_crit_dmg = critDmg->valueint;

  cJSON* lvl = cJSON_GetObjectItemCaseSensitive(obj, "lvl");
  if (lvl == NULL) handleError(ERR_DATA, FATAL, errMsg, "lvl");
  sw->lvl = lvl->valueint;

  cJSON* upgrades = cJSON_GetObjectItemCaseSensitive(obj, "upgrades");
  if (upgrades == NULL) handleError(ERR_DATA, FATAL, errMsg, "upgrades");
  sw->upgrades = upgrades->valueint;

  cJSON* durability = cJSON_GetObjectItemCaseSensitive(obj, "durability");
  if (durability == NULL) handleError(ERR_DATA, FATAL, errMsg, "durability");
  sw->durability = durability->valueint;

  return sw;
}

static Armor* createArmor(cJSON* obj) {
  const char* errMsg = "Could not find data for armor %s!\n";

  Armor* armor = (Armor*) malloc(sizeof(Armor));
  if (armor == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for armor!\n");

  cJSON* name = cJSON_GetObjectItemCaseSensitive(obj, "name");
  if (name == NULL) handleError(ERR_DATA, FATAL, errMsg, "name");
  armor->name = (char*) malloc(strlen(name->valuestring) + 1);
  if (armor->name == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for armor name!\n");
  strcpy(armor->name, name->valuestring);

  cJSON* type = cJSON_GetObjectItemCaseSensitive(obj, "type");
  if (type == NULL) handleError(ERR_DATA, FATAL, errMsg, "type");
  armor->type = type->valueint;

  cJSON* acc = cJSON_GetObjectItemCaseSensitive(obj, "acc");
  if (acc == NULL) handleError(ERR_DATA, FATAL, errMsg, "acc");
  armor->acc = acc->valueint;

  cJSON* def = cJSON_GetObjectItemCaseSensitive(obj, "def");
  if (def == NULL) handleError(ERR_DATA, FATAL, errMsg, "def");
  armor->def = def->valueint;

  cJSON* lvl = cJSON_GetObjectItemCaseSensitive(obj, "lvl");
  if (lvl == NULL) handleError(ERR_DATA, FATAL, errMsg, "lvl");
  armor->lvl = lvl->valueint;

  return armor;
}

static HPKit* createHPKit(cJSON* obj) {
  HPKit* hpKit = (HPKit*) malloc(sizeof(HPKit));
  if (hpKit == NULL) handleError(ERR_MEM, FATAL, "Could not allocate for HP Kit!\n");

  cJSON* type = cJSON_GetObjectItemCaseSensitive(obj, "type");
  if (type == NULL) handleError(ERR_DATA, FATAL, "Could not find data for HP Kit type!\n");
  hpKit->type = type->valueint;

  cJSON* desc = cJSON_GetObjectItemCaseSensitive(obj, "desc");
  if (desc == NULL) handleError(ERR_DATA, FATAL, "Could not find data for HP Kit description!\n");
  hpKit->desc = (char*) malloc(strlen(desc->valuestring) + 1);
  if (hpKit->desc == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for HP Kit description!\n");
  strcpy(hpKit->desc, desc->valuestring);

  return hpKit;
}

static Upgrade* createUpgrade(cJSON* obj) {
  Upgrade* upgrade = (Upgrade*) malloc(sizeof(Upgrade));
  if (upgrade == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for upgrade material!\n");

  cJSON* rank = cJSON_GetObjectItemCaseSensitive(obj, "rank");
  if (rank == NULL) handleError(ERR_DATA, FATAL, "Could not find data for upgrade rank!\n");
  upgrade->rank = rank->valueint;

  cJSON* type = cJSON_GetObjectItemCaseSensitive(obj, "type");
  if (type == NULL) handleError(ERR_DATA, FATAL, "Could not find data for upgrade type!\n");
  upgrade->type = type->valueint;

  cJSON* desc = cJSON_GetObjectItemCaseSensitive(obj, "desc");
  if (desc == NULL) handleError(ERR_DATA, FATAL, "Could not find data for upgrade description!\n");
  upgrade->desc = (char*) malloc(strlen(desc->valuestring) + 1);
  if (upgrade->desc == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for upgrade description!\n");
  strcpy(upgrade->desc, desc->valuestring);

  return upgrade;
}

static Slime* createSlime(cJSON* obj) {
  Slime* slime = (Slime*) malloc(sizeof(Slime));
  if (slime == NULL) handleError(ERR_MEM, FATAL, "Could not allocate for slime!\n");

  cJSON* desc = cJSON_GetObjectItemCaseSensitive(obj, "desc");
  if (desc == NULL) handleError(ERR_DATA, FATAL, "Could not find data for slime description!\n");
  slime->desc = (char*) malloc(strlen(desc->valuestring) + 1);
  if (slime->desc == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for slime description!\n");
  strcpy(slime->desc, desc->valuestring);

  return slime;
}

/**
 * Creates an item object given tie cJSON object.
 * @param obj The raw item data
 * @param type The type of item
 * @return The item
 */
Item* createItem(cJSON* obj, item_t type) {
  Item* item = (Item*) malloc(sizeof(Item));
  if (item == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for item!\n");

  item->type = type;

  cJSON* _count = cJSON_GetObjectItemCaseSensitive(obj, "count");
  if (_count == NULL) handleError(ERR_DATA, FATAL, "Could not get count data!\n");
  item->count = _count->valueint;

  cJSON* objItem = cJSON_GetObjectItemCaseSensitive(obj, "item");

  switch (type) {
    case SOULWEAPON_T:
      item->_item = createSoulWeapon(objItem);
      break;
    case HELMET_T:
    case SHOULDER_GUARD_T:
    case CHESTPLATE_T:
    case BOOTS_T:
      item->_item = createArmor(objItem);
    case HP_KITS_T:
      item->_item = createHPKit(objItem);
    case WEAPON_UPGRADE_MATERIALS_T:
    case ARMOR_UPGRADE_MATERIALS_T:
      item->_item = createUpgrade(objItem);
    case SLIME_T:
      item->_item = createSlime(objItem);
    default:
      break;
  }  

  return item;
}

/**
 * Randomly selects an item from the table.
 * @param table The table to select from
 * @return The selected item
 */
static Item* selectLoot(cJSON* table) {
  int len = cJSON_GetArraySize(table);

  if (len == 0) return NULL; // No items in table

  int i = rand() % len;

  cJSON* item = cJSON_GetArrayItem(table, i);

  if (item == NULL) {
    fprintf(stderr, "Error! Could not get item!\n");
    exit(-1);
  }

  cJSON* itemType = cJSON_GetObjectItemCaseSensitive(item, "type");
  item_t type = itemType->valueint;

  return createItem(item, type);
}


Enemy* initEnemy(cJSON* obj) {
  const char* errMsg = "Could not find data for enemy %s!\n";

  Enemy* enemy = (Enemy*) malloc(sizeof(Enemy));
  if (enemy == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for enemy!\n");

  cJSON* name = cJSON_GetObjectItemCaseSensitive(obj, "name");
  if (name == NULL) handleError(ERR_DATA, FATAL, errMsg, "name");
  enemy->name = (char*) malloc(strlen(name->valuestring));
  if (enemy->name == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for enemy name!\n");
  strcpy(enemy->name, name->valuestring);

  cJSON* xpPoints = cJSON_GetObjectItemCaseSensitive(obj, "xpPoints");
  if (xpPoints == NULL) handleError(ERR_DATA, FATAL, errMsg, "xp points");
  enemy->xpPoints = xpPoints->valueint;

  cJSON* hp = cJSON_GetObjectItemCaseSensitive(obj, "hp");
  if (hp == NULL) handleError(ERR_DATA, FATAL, errMsg, "hp");
  enemy->hp = hp->valueint;

  cJSON* lvl = cJSON_GetObjectItemCaseSensitive(obj, "lvl");
  if (lvl == NULL) handleError(ERR_DATA, FATAL, errMsg, "lvl");
  enemy->lvl = lvl->valueint;


  cJSON* stats = cJSON_GetObjectItemCaseSensitive(obj, "stats");

  enemy->stats = (Stats*) malloc(sizeof(Stats));
  if (enemy->stats == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for enemy stats!\n");

  cJSON* atk = cJSON_GetObjectItemCaseSensitive(stats, "atk");
  if (atk == NULL) handleError(ERR_DATA, FATAL, errMsg, "atk");
  enemy->stats->ATK = atk->valueint;

  cJSON* def = cJSON_GetObjectItemCaseSensitive(stats, "def");
  if (def == NULL) handleError(ERR_DATA, FATAL, errMsg, "def");
  enemy->stats->DEF = def->valueint;

  cJSON* acc = cJSON_GetObjectItemCaseSensitive(stats, "acc");
  if (acc == NULL) handleError(ERR_DATA, FATAL, errMsg, "acc");
  enemy->stats->ACC = acc->valueint;

  cJSON* atkCrit = cJSON_GetObjectItemCaseSensitive(stats, "atk_crit");
  if (atkCrit == NULL) handleError(ERR_DATA, FATAL, errMsg, "atk crit");
  enemy->stats->ATK_CRIT = atkCrit->valuedouble;

  cJSON* critDmg = cJSON_GetObjectItemCaseSensitive(stats, "akt_crit_dmg");
  if (critDmg == NULL) handleError(ERR_DATA, FATAL, errMsg, "atk crit dmg");
  enemy->stats->ATK_CRIT_DMG = critDmg->valueint;

  return enemy;
}

Boss* initBoss(cJSON* obj) {
  const char* errMsg = "Could not find data for boss %s!\n";

  Boss* boss = (Boss*) malloc(sizeof(Boss));
  if (boss == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for boss!\n");

  cJSON* name = cJSON_GetObjectItemCaseSensitive(obj, "name");
  if (name == NULL) handleError(ERR_DATA, FATAL, errMsg, "name");
  boss->base.name = (char*) malloc(strlen(name->valuestring));
  if (boss->base.name == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for boss name!\n");
  strcpy(boss->base.name, name->valuestring);

  cJSON* xpPoints = cJSON_GetObjectItemCaseSensitive(obj, "xpPoints");
  if (xpPoints == NULL) handleError(ERR_DATA, FATAL, errMsg, "xp points");
  boss->base.xpPoints = xpPoints->valueint;

  cJSON* hp = cJSON_GetObjectItemCaseSensitive(obj, "hp");
  if (hp == NULL) handleError(ERR_DATA, FATAL, errMsg, "hp");
  boss->base.hp = hp->valueint;

  cJSON* lvl = cJSON_GetObjectItemCaseSensitive(obj, "lvl");
  if (lvl == NULL) handleError(ERR_DATA, FATAL, errMsg, "lvl");
  boss->base.lvl = lvl->valueint;

  cJSON* stats = cJSON_GetObjectItemCaseSensitive(obj, "stats");

  boss->base.stats = (Stats*) malloc(sizeof(Stats));
  if (boss->base.stats == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for boss stats!\n");

  cJSON* atk = cJSON_GetObjectItemCaseSensitive(stats, "atk");
  if (atk == NULL) handleError(ERR_DATA, FATAL, errMsg, "atk");
  boss->base.stats->ATK = atk->valueint;

  cJSON* def = cJSON_GetObjectItemCaseSensitive(stats, "def");
  if (def == NULL) handleError(ERR_DATA, FATAL, errMsg, "def");
  boss->base.stats->DEF = def->valueint;

  cJSON* acc = cJSON_GetObjectItemCaseSensitive(stats, "acc");
  if (acc == NULL) handleError(ERR_DATA, FATAL, errMsg, "acc");
  boss->base.stats->ACC = acc->valueint;

  cJSON* atkCrit = cJSON_GetObjectItemCaseSensitive(stats, "atk_crit");
  if (atkCrit == NULL) handleError(ERR_DATA, FATAL, errMsg, "atk crit");
  boss->base.stats->ATK_CRIT = atkCrit->valuedouble;

  cJSON* critDmg = cJSON_GetObjectItemCaseSensitive(stats, "akt_crit_dmg");
  if (critDmg == NULL) handleError(ERR_DATA, FATAL, errMsg, "atk crit dmg");
  boss->base.stats->ATK_CRIT_DMG = critDmg->valueint;


  cJSON* gear = cJSON_GetObjectItemCaseSensitive(obj, "gear");
  if (gear == NULL) handleError(ERR_DATA, FATAL, errMsg, "gear");

  cJSON* sw = cJSON_GetObjectItemCaseSensitive(gear, "sw");
  if (sw == NULL) handleError(ERR_DATA, FATAL, errMsg, "gear soulweapon");
  boss->gearDrop.sw = createSoulWeapon(sw);

  cJSON* helmet = cJSON_GetObjectItemCaseSensitive(gear, "helmet");
  if (helmet == NULL) handleError(ERR_DATA, FATAL, errMsg, "gear helmet");
  boss->gearDrop.helmet = createArmor(helmet);

  cJSON* guard = cJSON_GetObjectItemCaseSensitive(gear, "guard");
  if (guard == NULL) handleError(ERR_DATA, FATAL, errMsg, "gear shoulder guard");
  boss->gearDrop.guard = createArmor(guard);

  cJSON* chestplate = cJSON_GetObjectItemCaseSensitive(gear, "chestplate");
  if (chestplate == NULL) handleError(ERR_DATA, FATAL, errMsg, "gear chestplate");
  boss->gearDrop.chestplate = createArmor(chestplate);

  cJSON* boots = cJSON_GetObjectItemCaseSensitive(gear, "boots");
  if (boots == NULL) handleError(ERR_DATA, FATAL, errMsg, "gear boots");
  boss->gearDrop.boots = createArmor(boots);

  return boss;
}

static Enemy* selectEnemy(cJSON* table) {
  int len = cJSON_GetArraySize(table);

  if (len == 0) return NULL; // No items in table

  int i = rand() % len;

  cJSON* enemy = cJSON_GetArrayItem(table, i);

  if (enemy == NULL) {
    fprintf(stderr, "Error! Could not get enemy!\n");
    exit(-1);
  }

  return initEnemy(enemy);
}

/**
 * Given a cJSON room, it creates a Room structure using its data
 * @param _room The cJSON room structure
 * @return The new Room
 */
static Room* createRoom(cJSON* _room) {
  Room* room = (Room*) malloc(sizeof(Room));
  room->enemy.enemy = NULL;
  room->loot = NULL;

  cJSON* info = cJSON_GetObjectItemCaseSensitive(_room, "info");
  cJSON* hasBoss = cJSON_GetObjectItemCaseSensitive(_room, "hasBoss");
  cJSON* exits = cJSON_GetObjectItemCaseSensitive(_room, "exits");
  cJSON* lootTable = cJSON_GetObjectItemCaseSensitive(_room, "loot");
  cJSON* enemyTable = cJSON_GetObjectItemCaseSensitive(_room, "enemy");


  room->id = (char) atoi(_room->string);

  room->hasBoss = (bool)hasBoss->valueint;

  room->info = (char*) malloc(sizeof(char) * strlen(info->valuestring) + 1);
  strcpy(room->info, info->valuestring);

  Item* loot = selectLoot(lootTable);
  room->loot = loot;

  bool _hasBoss = (bool)hasBoss->valueint;

  if (_hasBoss) {
    cJSON* _boss = cJSON_GetArrayItem(enemyTable, 0);
    if (_boss == NULL) {
      fprintf(stderr, "Could not get boss data!\n");
      exit(-1);
    }
    Boss* boss = initBoss(_boss);
    room->enemy.boss = boss;
  } else {
    Enemy* enemy = selectEnemy(enemyTable);
    room->enemy.enemy = enemy;
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
  deleteItem(room->loot->_item);
  deleteEnemyFromMap(room);
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