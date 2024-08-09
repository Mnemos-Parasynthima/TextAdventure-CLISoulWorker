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
#include "Error.h"
#include "Keyboard.h"


SoulWorker* player;
Maze* maze;


void loop() {
  Room* currRoom = player->room;
  Commands choice;
  
  printf("You are in %s...\n", currRoom->info);

  while (true) {
    if (currRoom->loot != NULL) {
      printf("You found %s! Do you want to add it to your inventory? (y|n) ", currRoom->loot);
      choice = getchar();
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