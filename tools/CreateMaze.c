#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>

#include "../headers/Error.h"
#include "../headers/Colors.h"
#include "../headers/cJSON.h"

// Needs to match up with Misc.h!!!

typedef enum {
  HELMET,
  SHOULDER_GUARD,
  CHESTPLATE,
  BOOTS
} armor_t;

typedef enum {
  NONE,
  SOULWEAPON_T,
  HELMET_T,
  SHOULDER_GUARD_T,
  CHESTPLATE_T,
  BOOTS_T,
  HP_KITS_T,
  WEAPON_UPGRADE_MATERIALS_T,
  ARMOR_UPGRADE_MATERIALS_T,
  SLIME_T
} item_t;


/**
 * Converts integer n into a character string.
 * @param n The integer
 * @param buffer The buffer to hold the string
 * @return The integer as a string
 */
extern str itoa(int n, char* buffer, int radix);

/**
 * Handles errors regarding cJSON objects.
 * @param obj The cJSON object
 * @param type The JSON object tag
 * @return NULL
 */
static void createError(cJSON* obj, const str type) {
  cJSON_Delete(obj);

  handleError(ERR_DATA, WARNING, "Could not create JSON for %s!\n", type);

  exit(-1);
}

/**
 * Used by qsort to compare file names based on their id
 * @param a File name a
 * @param b File name b
 * @return 0 if equal, otherwise, they are different
 */
static int compare(const void* a, const void* b) {
  int aID = atoi(*((str*) a));
  int bID = atoi(*((str*) b));

  return aID - bID;
}

/**
 * Gets the room files from the output room directory, placing opened files in the array,
 * sorted by their ID, where 0.room is the first element.
 * @param roomCount How many rooms to open and place
 * @return Array of opened room files
 */
static FILE** getRoomFiles(int* roomCount) {
  const str ROOMS_DIR = "./out/rooms";

  DIR* roomsDir = opendir(ROOMS_DIR);

  // Get the number of room files
  int count = 0;
  struct dirent* entry = readdir(roomsDir);
  while (entry != NULL) {
    if (entry->d_type == DT_REG) count++;
    entry = readdir(roomsDir);
  }
  *roomCount = count;
  printf("%sFound %d rooms...%s\n", PURPLE, count, RESET);

  rewinddir(roomsDir);

  FILE** rooms = (FILE**) malloc(sizeof(FILE*) * count);
  if (!rooms) handleError(ERR_MEM, FATAL, "Could not allocate memory for room files!\n");

  // Read all the entries into an array, placing the names and skipping "." and ".."
  // The array will be used to open the files, so they need to be in order
  // Thus array is to be sorted
  str* names = (str*) malloc(sizeof(str) * count);
  if (!names) handleError(ERR_MEM, FATAL, "Could not allocate memory for room names!\n");
  str* _temp = names;

  entry = readdir(roomsDir);
  while (entry != NULL) {
    if (entry->d_type == DT_DIR) {
      entry = readdir(roomsDir);
      continue;
    }
    *_temp = &entry->d_name;
    entry = readdir(roomsDir);
    _temp++;
    // No need to check if filename is valid
    // Will be done later on
  }

  qsort(names, count, sizeof(str), compare);

  // Fill rooms array with open files for all room files
  // Reminder to close the files at the end
  str filename = (str) malloc(19);
  for (int i = 0; i < count; i++) {
    // printf("%si: %d%s\n", YELLOW, i, RESET);
    printf("Placing file %s\n", *names);

    str temp = (str) realloc(filename, 13 + strlen(*names));
    if (!temp) handleError(ERR_MEM, FATAL, "Could not allocate memory for filename!\n");
    filename = temp;

    sprintf(filename, "%s/%s", ROOMS_DIR, *names);

    rooms[i] = fopen(filename, "r");
    if (!rooms[i]) handleError(ERR_IO, FATAL, "Could not open room file!\n");

    entry = readdir(roomsDir);
    names++;
  }
  // printf("%sFinished placing%s\n", YELLOW, RESET);
  free(filename);
  closedir(roomsDir);

  return rooms;
}

/**
 * Gets the contents of the provided file. Assumed to be in a single line.
 * @param infoFile The file containing the description for the room
 * @return The file contents
 */
static str getInfo(str infoFile) {
  int infoFileLen = strlen(infoFile);
  str filename = (str) malloc(8 + infoFileLen);
  if (!filename) handleError(ERR_MEM, FATAL, "Could not allocate memory for info file!\n");

  sprintf(filename, "./info/%s", infoFile);

  FILE* file = fopen(filename, "r");
  if (!file) handleError(ERR_IO, FATAL, "Could not open info file %s!\n", infoFile);

  free(filename);

  str info = NULL;

  size_t n;
  // Assuming entire info is in a single line
  size_t read = getline(&info, &n, file);
  *(info + read - 1) = '\0';

  return info;
}

/**
 * Converts the type in string to the type in number.
 * @param _str The type as a string
 * @return The type as a number
 */
static item_t strToType(str _str) {
  item_t type;

  if (strcmp(_str, "soulweapon") == 0) type = SOULWEAPON_T;
  else if (strcmp(_str, "helmet") == 0) type = HELMET_T;
  else if (strcmp(_str, "shoulder_guard") == 0) type = SHOULDER_GUARD_T;
  else if (strcmp(_str, "chestplate") == 0) type = CHESTPLATE_T;
  else if (strcmp(_str, "boots") == 0) type = BOOTS_T;
  else if (strcmp(_str, "hp_kit") == 0) type = HP_KITS_T;
  else if (strcmp(_str, "weapon_upgrade") == 0) type = WEAPON_UPGRADE_MATERIALS_T;
  else if (strcmp(_str, "armor_upgrade") == 0) type = ARMOR_UPGRADE_MATERIALS_T;
  else type = SLIME_T;

  return type;
}

/**
 * Fills the JSON object with the soulweapon data from the file.
 * @param item The JSON object to fill out
 * @param file The file
 */
static void createSoulWeapon(cJSON* item, FILE* file) {
  str line = NULL;
  size_t n;

  // TODO: Check for null

  ssize_t read = getline(&line, &n, file);
  *(line + read - 1) = '\0';

  cJSON* name = cJSON_AddStringToObject(item, "name", line);
  // 

  int stat;
  float statf;

  fscanf(file, "%d", &stat);
  cJSON* atk = cJSON_AddNumberToObject(item, "atk", stat);
  //

  fscanf(file, "%d", &stat);
  cJSON* acc = cJSON_AddNumberToObject(item, "acc", stat);
  //

  fscanf(file, "%f", &statf);
  cJSON* atkCrit = cJSON_AddNumberToObject(item, "atk_crit", statf);
  //

  fscanf(file, "%d", &stat);
  cJSON* atkCritDmg = cJSON_AddNumberToObject(item, "atk_crit_dmg", stat);
  //

  fscanf(file, "%d", &stat);
  cJSON* lvl = cJSON_AddNumberToObject(item, "lvl", stat);
  //

  cJSON* upgrades = cJSON_AddNumberToObject(item, "upgrades", 0);
  //

  cJSON* durability = cJSON_AddNumberToObject(item, "durability", 100);
  //
}

/**
 * Fills the JSON object with the armor data from the file, depending on the type.
 * @param item The JSON object to fill out
 * @param file The file
 * @param type The type of armor
 */
static void createArmor(cJSON* item, FILE* file, armor_t type) {
  str line = NULL;
  size_t n;

  ssize_t read = getline(&line, &n, file);
  *(line + read - 1) = '\0';

  // TODO: Check for null

  cJSON* name = cJSON_AddStringToObject(item, "name", line);
  // 

  int stat;

  cJSON* _type = cJSON_AddNumberToObject(item, "type", type);
  //

  fscanf(file, "%d", &stat);
  cJSON* acc = cJSON_AddNumberToObject(item, "acc", stat);
  //

  fscanf(file, "%d", &stat);
  cJSON* def = cJSON_AddNumberToObject(item, "def", stat);
  //

  fscanf(file, "%d", &stat);
  cJSON* lvl = cJSON_AddNumberToObject(item, "lvl", stat);
  //
}

/**
 * Adds a loot object to the loot array for the provided item file.
 * @param root The root JSON file
 * @param lootArr The JSON loot array object
 * @param item The loot item to add, indicating the file to use
 */
static void addLoot(cJSON* root, cJSON* lootArr, str item) {
  // item: [type]::[id]

  // Parse item into [type] and [id]
  str type = strtok(item, "::");
  str id = strtok(NULL, "::");

  // Open file ./out/items/[type].item
  int typeLen = strlen(type);
  str filename = malloc(18 + typeLen);
  if (!filename) handleError(ERR_MEM, FATAL, "Could not allocate memory for filename for loot item!\n");

  sprintf(filename, "./out/items/%s.item", type);

  FILE* itemFile = fopen(filename, "r");
  if (!itemFile) handleError(ERR_IO, FATAL, "Could not open loot item file!\n");

  // Check for empty
  char c = fgetc(itemFile);
  if (c == EOF) handleError(ERR_DATA, FATAL, "Empty item file for %s!\n", filename);
  else ungetc(c, itemFile);

  // Loop until we get to the desired item

  str line = NULL;
  size_t n;
  ssize_t read = getline(&line, &n, itemFile);
  *(line + read - 1) = '\0';

  // In case the target is the very first item
  // Skip through the looping-check
  if (strcmp(line, id) != 0) {
    // Skip through all the lines until END_ITEM
    // Check if the line after time has the target ID
    // If no, keep skipping until target
    // If yes, exit and start processing data from that point

    bool itemFound = false;

    while (!feof(itemFile)) {
      if (strncmp(line, "END_ITEM", 8) == 0) {
        read = getline(&line, &n, itemFile);
        *(line + read - 1) = '\0';
        
        // If we reached the last end of item
        if (feof(itemFile)) break;

        // line now should have the id, check for target
        if (strcmp(line, id) == 0) { itemFound = true; break; }
      }
      getline(&line, &n, itemFile);
    }

    if (!itemFound) handleError(ERR_DATA, FATAL, "Could not find item %s of %s!\n", id, type);
  }

  // file pointer should point to the next line after ID
  // start processing the individual data (depends greatly on type)
  // For all items, the data immediately after is the count, so add that before
  // Type is also known so add it

  cJSON* _loot = cJSON_CreateObject();
  if (!_loot) { createError(root, "loot"); return; }
  if (!cJSON_AddItemToArray(lootArr, _loot)) return;

  // TODO: Check for null

  read = getline(&line, &n, itemFile);
  *(line + read - 1) = '\0';
  cJSON* count = cJSON_AddNumberToObject(_loot, "count", atoi(line));
  // if (!count) {  }

  // Move the following section to its own function?????

  item_t itemType = strToType(type);

  cJSON* _type = cJSON_AddNumberToObject(_loot, "type", itemType);

  cJSON* lootItem = cJSON_AddObjectToObject(_loot, "item");
  if (!lootItem) { createError(root, "loot item"); return; }

  switch (itemType) {
    case SOULWEAPON_T:
      createSoulWeapon(lootItem, itemFile);
      break;
    case HELMET_T:
    case SHOULDER_GUARD_T:
    case CHESTPLATE_T:
    case BOOTS_T:
      createArmor(lootItem, itemFile, itemType - 2);
      break;
    case HP_KITS_T:
      read = getline(&line, &n, itemFile);
      *(line + read - 1) = '\0';

      cJSON* hpKitT = cJSON_AddNumberToObject(lootItem, "type", *line - 0x30);
      // if (!hpKitT)

      read = getline(&line, &n, itemFile);
      *(line + read - 1) = '\0';

      cJSON* hpKitDesc = cJSON_AddStringToObject(lootItem, "description", line);
      // if (!hpKitDesc)
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
    case ARMOR_UPGRADE_MATERIALS_T:
      read = getline(&line, &n, itemFile);
      *(line + read - 1) = '\0';

      cJSON* upgradeRank = cJSON_AddNumberToObject(lootItem, "rank", *line - 0x30);
      // if (!upgradeRank)

      cJSON* upgradeType = cJSON_AddNumberToObject(lootItem, "type", itemType - 7);
      // if (!upgradeType)

      read = getline(&line, &n, itemFile);
      *(line + read - 1) = '\0';

      cJSON* upgradeDesc = cJSON_AddStringToObject(lootItem, "description", line);
      // if (!upgradeDesc)
      break;
    case SLIME_T:
      read = getline(&line, &n, itemFile);
      *(line + read - 1) = '\0';

      cJSON* slimeDesc = cJSON_AddStringToObject(lootItem, "description", line);
      // if (!slimeDesc)
    default:
      break;
  }

  free(filename);
  fclose(itemFile);
}

/**
 * Creates the JSON object for the provided room file.
 * @param root The root JSON object
 * @param id The id of the room
 * @param room The room file
 */
static void createRoom(cJSON* root, int id, FILE* room) {
  // Check if file is not empty
  char c = getc(room);
  if (c == EOF) {
    handleError(ERR_DATA, WARNING, "File %d is empty!\n", id);
    return;
  }
  ungetc(c, room);

  char buffer[2];

  printf("%sCreating room %d....%s\n", PURPLE, id, RESET);

  cJSON* roomObj = cJSON_AddObjectToObject(root, itoa(id, buffer, 10));
  if (!roomObj) createError(root, "room");

  str line = NULL;
  size_t n;
  ssize_t read;

  // Get the id
  read = getline(&line, &n, room);
  int _id = *line - 0x30;
  // Make sure the IDs match between in file and array
  if (id != _id) {
    printf("IDs do not match! File contained %d while at %dth iteration.\n", _id, id);
    exit(1);
  }

  cJSON* isEntry = cJSON_AddNumberToObject(roomObj, "isEntry", (id == 0) ? 1 : 0);
  if (!isEntry) createError(root, "isEntry");
  printf("Added isEntry tag with value of %d...\n", (id == 0) ? 1 : 0);

  // Get storyfile
  // if (feof(room)) 
  // Handle when no line to get
  // Maybe it should leave the data json as is and continue
  // or stop and exit
  // or delete the room and continue to other rooms
  read = getline(&line, &n, room);
  *(line + read - 1) = '\0';

  cJSON* storyfile = cJSON_AddStringToObject(roomObj, "storyfile", line);
  if (!storyfile) createError(root, "storyfile");
  printf("Added storyfile tag with value of %s\n", line);

  // Get info
  read = getline(&line, &n, room);
  *(line + read - 1) = '\0';

  str _info = getInfo(line);
  cJSON* info = cJSON_AddStringToObject(roomObj, "info", _info);
  if (!info) createError(root, "info");
  printf("Added info tag\n");

  // Get exits
  read = getline(&line, &n, room);
  *(line + read - 1) = '\0';

  cJSON* exits = cJSON_AddArrayToObject(roomObj, "exits");
  if (!exits) createError(root, "exits");
  for (int i = 0; i < 4; i++) {
    int num = 0;
    if (*line == '-') {
      num = ~(*(line + 1) - 0x30) + 1;
      line += 3;
    } else {
      num = *line - 0x30;
      line += 2;
      // For the last number, after this, line will point to a mem addr
      // that may not be valid, but it should not access it
      // Actually, it may point to the second null char
    }
    cJSON* exit = cJSON_CreateNumber(num);
    if (!exit) createError(root, "exit");

    if (!cJSON_AddItemToArray(exits, exit)) createError(root, "exit in exits");
    printf("Added exit %d...", num);
  }
  printf("\n");
  line = NULL;

  // Get if boss
  read = getline(&line, &n, room);
  *(line + read - 1) = '\0';

  bool _hasBoss = atoi(line);
  cJSON* hasBoss = cJSON_AddNumberToObject(roomObj, "hasBoss", _hasBoss);
  if (!hasBoss) createError(root, "hasBoss");
  printf("Added hasBoss tag with value of %s\n", line);

  // Get loot
  read = getline(&line, &n, room);
  *(line + read - 1) = '\0';

  cJSON* lootArr = cJSON_AddArrayToObject(roomObj, "loot");
  if (!lootArr) createError(root, "loot");

  // If the line has "-1", it means no loot, so keep array empty
  // Otherwise, line is an array of [type]::[id]
  // Parse and iterate through them

  // Future me to do:
  // Currently, the appropriate file is opened and closed for each pair
  // In the case that there are multiple items of the same type (but diff id)
  // The files will re-open many times
  // They can also be spread out (not consecutive)
  // So maybe just parse every item, figure out what files to open,
  // and keep them open until all items are processed
  if (strncmp(line, "-1", 2) != 0) {
    // Example line is soulweapon::0,hp::3,boots::5

    str saveptr = NULL;
    str item = strtok_r(line, ",", &saveptr);
    while (item != NULL) {
      addLoot(root, lootArr, item);
      printf("Added loot...");
      item = strtok_r(NULL, ",", &saveptr);
    }
    printf("\n");
  } else printf("Added empty loot tag\n");

  // Get enemies
  read = getline(&line, &n, room);
  *(line + read - 1) = '\0';

  cJSON* enemies = cJSON_AddArrayToObject(roomObj, "enemy");
  if (!enemies) createError(root, "enemy");

  // If the line has "-1", it means no enemies, so keep array empty
  // if (strncmp(line, "-1", 2) != 0) {
  //   // Line is not "-1", meaning it has file to enemies
  //   // Rooms that only have a boss will only have one item
  //   // Pass in to the function that it has a boss
  //   addEnemies(root, enemies, line);
  // } 
}

void saveCopy(str maze, str filename) {
  // [name].json to ./history/[name]-[time-date].json

  time_t timer;
  time(&timer);
  struct tm* info = localtime(&timer);

  // Format: [month]_[day]_[year]-[hour]_[minute]_[second]
  int datetimeLen = 19;
  char buffer[datetimeLen + 1];
  strftime(buffer, 64, "%m_%d_%Y-%H_%M_%S", info);

  int filenameLen = strlen(filename);
  int newFilenameLen = 10 + filenameLen + 1 + datetimeLen + 1;

  str newFilename = (str) malloc(newFilenameLen);
  if (!newFilename) {
    handleError(ERR_MEM, WARNING, "Could not allocate memory for archiving filename!\n");
    return;
  }

  int nameLen = filenameLen - 5;

  // Null out the '.' on the extension to ignore the .json part
  *(filename + nameLen) = '\0';

  sprintf(newFilename, "./history/%s-%s.json", filename, buffer);

  FILE* file = fopen(newFilename, "w");
  if (!file) {
    handleError(ERR_IO, WARNING, "Could not create file!\n");
    free(newFilename);
  }

  int written = fprintf(file, "%s", maze);
  if (written <= 0) {
    handleError(ERR_IO, WARNING, "Could not save maze!\n");
    free(newFilename);
  }
  printf("%sCopy saved to %s!%s\n", GREEN, newFilename, RESET);
}

int main(int argc, char const* argv[]) {
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "usage: CreateMaze name_of_maze [archive (-a)]\n");
    return 1;
  }

  bool archive = false;

  if (argc == 3) {
    str _archive = argv[2];
    if (strncmp(_archive, "-a", 2) == 0) archive = true;
  }

  str _mazeName = argv[1];

  int roomCount = 0;
  FILE** rooms = getRoomFiles(&roomCount);

  cJSON* root = cJSON_CreateObject();

  cJSON* mazeName = cJSON_AddStringToObject(root, "name", _mazeName);
  if (!mazeName) createError(root, "maze name");

  // Create each room
  for (int i = 0; i < roomCount; i++) {
    createRoom(root, i, rooms[i]);
  }
  printf("%sRooms have been created!%s\n", GREEN, RESET);

  str maze = cJSON_Print(root);
  if (!maze) handleError(ERR_MEM, FATAL, "Could not create json string!\n");
  cJSON_Delete(root);

  int mazeNameLength = strlen(_mazeName);
  str filename = (str) malloc(6 + mazeNameLength);
  sprintf(filename, "%s.json", _mazeName);

  FILE* file = fopen(filename, "w");
  if (!file) handleError(ERR_IO, FATAL, "Could not create file!\n");

  int written = fprintf(file, "%s", maze);
  if (written <= 0) handleError(ERR_IO, FATAL, "Could not save maze!\n");
  
  printf("%sSaved to %s!%s\n", GREEN, filename, RESET);

  fclose(file);

  // Have a copy be saved to history
  if (archive) saveCopy(maze, filename);

  free(filename);
  cJSON_free(maze); // ????

  for (int i = 0; i < roomCount; i++) {
    fclose(rooms[i]);
  }

  return 0;
}