#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN64
#include "unistd.h" // For compiling in Windows MSVC
#include <windows.h> // Using sleep(). Note, this sleep takes in ms
#else
#include <unistd.h> // Note, the sleep takes in sec
#endif

#include "SoulWorker.h"
#include "Setup.h"
#include "Maze.h"
#include "Error.h"
#include "Keyboard.h"
#include "SaveLoad.h"


SoulWorker* player;
Maze* maze;


/**
 * Wrapper around sleep function for Windows and Linux.
 * @param ms Amount of milliseconds to sleep for
 */
void ssleep(int ms) {
  #ifdef _WIN64
    Sleep(ms);
  #else
    usleep(ms * 1000);
  #endif
}

/**
 * Detects whether there is a saved game so the player can resume.
 * @return True if a saved game exists, false otherwise
 */
static bool detectSave() {
  FILE* mapSave = fopen("./data/saves/maps/map_save.json", "r");
  FILE* playerSave = fopen("./data/saves/player_save.json", "r");

  if (mapSave != NULL && playerSave != NULL) {
    fclose(mapSave);
    fclose(playerSave);

    return true;
  }

  return false;
}

static void introStory() {
  FILE* intro = fopen("./data/story/intro.dat", "r");

  if (intro == NULL) handleError(ERR_IO, FATAL, "Could not open story!\n");

  char* line = NULL;
  size_t n = 0;

  printf("\n");
  while (!feof(intro)) {
    getline(&line, &n, intro);
    if (feof(intro)) break;
    ssleep(1000);
    printf("%s", line);
    line = NULL;
  }
  printf("\n\n");


  fclose(intro);
}


void loop() {
  Room* currRoom = player->room;
  Commands choice;
  
  printf("You are in %s...\n", currRoom->info);

  while (true) {
    if (currRoom->loot != NULL) {
      char* name = getItemName(currRoom->loot);
      printf("You found %s!", name);
      bool added = addToInv(player, currRoom->loot);
      if (added) removeItemFromMap(currRoom);

      // Since some strings have been alloc'd, free them
      switch (currRoom->loot->type) {
        case HP_KITS_T:
        case WEAPON_UPGRADE_MATERIALS_T:
        case ARMOR_UPGRADE_MATERIALS_T:
          free(name);
          name = NULL;
        default:
          break;
      }
    }

    if (!currRoom->hasBoss && currRoom->enemy.enemy != NULL) {
      printf("You found an enemy! It's a %s!\n", currRoom->enemy.enemy->name);
      // Fighting mechanics
      deleteEnemyFromMap(currRoom);
    }

    printf("What are you going to do?... ");
    choice = getchar();
    choice = tolower(choice);
    FLUSH()

    while(!performAction(choice, player)) {
      printf("That is not an action. Try again! For a list of acceptable actions, type 'h'!\n");
      choice = getchar();
      choice = tolower(choice);
      FLUSH()
    };

    currRoom = player->room;
  }
}


int main(int argc, char const *argv[]) {
  printf("Welcome to the Cloudream Adventure!\n");

  bool saveExists = detectSave();

  if (saveExists) {
    printf("A save has been detected! Do you want to load that save? (yes|no) ");

    // char buff[5];
    // fgets(buff, sizeof(buff), stdin);
    char* buff = (char*) malloc(5); // max is 3 for "yes", plus newline, plus null
    fgets(buff, 5, stdin);

    char* in = buff; // better name
    for(;*buff != '\0';buff++) *buff = tolower(*buff);

    if (strncmp(in, "yes", 3) == 0) {
      printf("Loading save...\n");
      loadGame();

      printf("Welcome back to Cloudream, %s!\n", player->name);

      loop();
    } else if (strncmp(in, "no", 2) == 0) {
      printf("Starting a new game...\n");
    } else {
      printf("Invalid decision. Starting a new game.\n");
    }
  }

  maze = initMaze("./data/maps/map.json");

  printf("Enter your name: ");

  char* name = NULL;
  size_t n = 0;
  ssize_t nameLen = getline(&name, &n, stdin);
  *(name + nameLen - 1) = '\0';

  player = initSoulWorker(name);
  player->room = maze->entry;

  // printf("");
  // printf("Is\n\t%s\t\tyour name?...\n", name);
  // Do a yes or no, repeat type of thing
  // printf("")

  printf("Hello, %s! Welcome to Cloudream!\n", (player->name));
  introStory();
  loop();


  return 0;
}