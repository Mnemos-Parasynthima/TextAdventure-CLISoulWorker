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
#define DELAY 1000
#else
#include <unistd.h> // Note, the sleep takes in sec
#define DELAY 1
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
    sleep(DELAY);
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
      printf("You found %s! Do you want to add it to your inventory? (y|n) ", currRoom->loot);
      choice = getchar();
      choice = tolower(choice);
      FLUSH()

      if (choice == 'y') {
        bool added = addToInv(player, *(currRoom->loot));
        if (added) removeItemFromMap(currRoom);
        // viewInventory(player);
      } else {
        printf("You did not add the item.\n");
      }
    }

    if (currRoom->enemy != NULL) {
      printf("You found an enemy! It's a %s!\n", currRoom->enemy);
      // Fighting mechanics
      removeEnemyFromMap(currRoom);
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