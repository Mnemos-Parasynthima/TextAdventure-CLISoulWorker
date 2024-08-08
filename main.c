#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef _WIN64
#include "unistd.h" // For compiling in Windows MSVC
#else
#include <unistd.h>
#endif

#include "SoulWorker.h"
#include "Setup.h"
#include "Maze.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define CHAR_TO_INDEX(c) \
    ((c) == 'N' ? 0 : \
    (c) == 'E' ? 1 : \
    (c) == 'S' ? 2 : \
    3)
#define VALID_INPUT(c) ((c) == 'N' || (c) == 'E' || (c) == 'S' || (c) == 'W')

SoulWorker* player;
Maze* maze;



bool validateInput(Room* room, int choice) {
  const char* DIR_CLOSED = "That direction is closed. Try another direction!\n";

  if ((choice == 'N' && room->exits[0] == (void*)NO_EXIT) ||
      (choice == 'E' && room->exits[1] == (void*)NO_EXIT) ||
      (choice == 'S' && room->exits[2] == (void*)NO_EXIT) ||
      (choice == 'W' && room->exits[3] == (void*)NO_EXIT)) {

    printf("%s", DIR_CLOSED);

    return false;
  }

  if (!VALID_INPUT(choice)) {
    printf("Not a direction. Try again.\n");

    return false;
  }

  return true;
}


void loop() {
  Room* currRoom = player->room;
  int choice;

  
  while (true) {
    printf("You are in the %s...\n", currRoom->info);

    if (currRoom->loot != NULL) {
      printf("You found %s! Do you want to add it to your inventory? (y|n) ", currRoom->loot);
      choice = getchar();
      getchar();

      if (choice == 'y') {
        addToInv(player, *(currRoom->loot));
        removeItemFromMap(currRoom);
        viewInventory(player);
      } else {
        printf("You did not add the item.\n");
      }
    }

    if (currRoom->enemy != NULL) {
      printf("You found an enemy! It's a %s!\n", currRoom->enemy);
      // Fighting mechanics
      removeEnemyFromMap(currRoom);
    }


    printf("Which direction do you want to go? Type N for up, E for right, S for down, and W for left: ");
    choice = getchar();
    getchar();

    if (validateInput(currRoom, choice)) {
      printf("Entering room...\n");
      currRoom = currRoom->exits[CHAR_TO_INDEX(choice)];
    } else {

    }
  }
}




int main(int argc, char const *argv[]) {
  maze = initMaze();

  printf("Welcome to the Cloudream Adventure!\n");
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
  printf("You are a SoulWorker exploring the N-102 Lab in Candus City.\n");

  loop();


  return 0;
}
