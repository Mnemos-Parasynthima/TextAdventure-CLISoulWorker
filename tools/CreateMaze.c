#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "../headers/Error.h"
#include "../headers/cJSON.h"


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
 * 
 * @param roomCount 
 * @return 
 */
static FILE** getRoomFiles(int* roomCount) {
  DIR* roomsDir = opendir("./out/rooms");

  // Get the number of room files
  int count = 0;
  struct dirent* entry = readdir(roomsDir);
  while (entry != NULL) {
    if (entry->d_type == DT_REG) count++;
    entry = readdir(roomsDir);
  }
  *roomCount = count;

  rewinddir(roomsDir);

  // Skip the current and parent entry
  entry = readdir(roomsDir);
  while (entry->d_type == DT_DIR) entry = readdir(roomsDir);

  FILE** rooms = (FILE**) malloc(sizeof(FILE*) * count);
  if (!rooms) handleError(ERR_MEM, FATAL, "Could not allocate memory for room files!\n");

  // Fill rooms array with open files for all room files
  // Reminder to close the files at the end
  for (int i = 0; i < count; i++) {
    printf("Placing file %s\n", entry->d_name);
    rooms[i] = fopen(entry->d_name, "r");
    if (!rooms[i]) handleError(ERR_IO, FATAL, "Could not open room file!\n");

    entry = readdir(roomsDir);
  }

  closedir(roomsDir);

  return rooms;
}

/**
 * Gets the contents of the provided file.
 * @param infoFile 
 * @return 
 */
str getInfo(str infoFile) {
  int infoFileLen = strlen(infoFile);
  str filename = (str) malloc(8 + infoFileLen);
  if (!filename) handleError(ERR_MEM, FATAL, "Could not allocate memory for info file!\n");

  sprintf(filename, "./info/%s", infoFile);

  FILE* file = fopen(filename, "r");
  if (!file) handleError(ERR_IO, FATAL, "Could not open info file!\n");

  free(filename);

  str info = NULL;

  // PROCESS
  size_t n;
  // Assuming entire info is in a single line
  getline(&info, &n, file);
  *(info + n - 1) = '\0';

  return info;
}

/**
 * 
 * @param root The root JSON file
 * @param lootArr The JSON loot array object
 * @param item The loot item to add
 */
void addLoot(cJSON* root, cJSON* lootArr, str item) {
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

  // Loop until we get to the desired item
  // What type does not matter now as all items will have its ID after END_ITEM

  char c = fgetc(itemFile);
  if (c == EOF) handleError(ERR_DATA, FATAL, "Empty item file for %s!\n", filename);
  else ungetc(c, itemFile);

  str line = NULL;
  size_t n;
  getline(&line, &n, itemFile);
  *(line + n - 1) = '\0';

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
        getline(&line, &n, itemFile);
        *(line + n - 1) = '\0';
        
        // If we reached the last end of item
        if (feof(itemFile)) break;

        // line now should have the id, check for target
        if (strcmp(line, id) == 0) { itemFound = true; break; }
      }
      getline(&line, &n, itemFile);
    }

    if (!itemFound) handleError(ERR_DATA, FATAL, "Could not find the specified item ID!\n");
  }

  // file pointer should point to the next line after ID

  

  free(filename);
  fclose(itemFile);
}

/**
 * 
 * @param root 
 * @param id 
 */
void createRoom(cJSON* root, int id, FILE* room) {
  // Check if file is not empty
  char c = getchar();
  if (c == EOF) {
    handleError(ERR_DATA, WARNING, "File %d is empty!\n", id);
    return;
  }
  ungetc(c, room);

  char buffer[2];

  cJSON* roomObj = cJSON_AddObjectToObject(root, itoa(id, buffer, 10));
  if (!roomObj) createError(root, "room");

  str line;
  size_t n;

  // Get the id
  getline(&line, &n, room);
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
  getline(&line, &n, room);
  *(line + n - 1) = '\0';

  cJSON* storyfile = cJSON_AddStringToObject(roomObj, "storyfile", line);
  if (!storyfile) createError(root, "storyfile");
  printf("Added storyfile tag with value of %s", line);

  // Get info
  getline(&line, &n, room);
  *(line + n - 1) = '\0';

  str _info = getInfo(line);
  cJSON* info = cJSON_AddStringToObject(roomObj, "info", _info);
  if (!info) createError(root, "info");
  printf("Added info tag\n");

  // Get exits
  getline(&line, &n, room);
  *(line + n - 1) = '\0';

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
  getline(&line, &n, room);
  *(line + n - 1) = '\0';

  cJSON* hasBoss = cJSON_AddNumberToObject(roomObj, "hasBoss", atoi(line));
  if (!hasBoss) createError(root, "hasBoss");
  printf("Added hasBoss tag with value of %s\n", line);

  // Get loot
  getline(&line, &n, room);
  *(line + n - 1) = '\0';

  cJSON* loot = cJSON_AddArrayToObject(roomObj, "loot");
  if (!loot) createError(root, "loot");

  // If the line has "-1", it means no loot, so keep array empty
  // Otherwise, line is an array of [type]::[id]
  // Parse and iterate through them
  if (strncmp(line, "-1", 2) != 0) {
    // Example line is soulweapon::0,hp::3,boots::5

    str item = strtok(line, ",");
    while (item != NULL) {
      addLoot(root, loot, item);
      item = strtok(NULL, ",");
    }
  } else printf("Added empty loot tag\n");

  // Get enemies
  getline(&line, &n, room);
  *(line + n - 1) = '\0';

  cJSON* enemies = cJSON_AddArrayToObject(roomObj, "enemy");
  if (!enemies) createError(root, "enemy");

  // If the line has "-1", it means no enemies, so keep array empty
  // if (strncmp(line, "-1", 2) != 0) {
  //   // Line is not "-1", meaning it has file to enemies
  //   addEnemies(root, enemies, line);
  // } 
}


int main(int argc, char const* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: CreateMaze [name_of_maze]\n");
    return 1;
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

  str maze = cJSON_Print(root);
  if (!maze) handleError(ERR_MEM, FATAL, "Could not create json string!\n");
  cJSON_Delete(root);

  int mazeNameLength = strlen(_mazeName);
  str filename = (str) malloc(6 + mazeNameLength);
  sprintf(filename, "%s.json", _mazeName);

  FILE* file = fopen(filename, "w");
  if (!file) handleError(ERR_IO, FATAL, "Could not create file!\n");
  free(filename);

  int written = fprintf(file, "%s", maze);
  if (written <= 0) handleError(ERR_IO, FATAL, "Could not save maze!\n");

  fclose(file);

  // Have a copy be saved to history

  cJSON_free(maze); // ????

  for (int i = 0; i < roomCount; i++) {
    fclose(rooms[i]);
  }

  return 0;
}