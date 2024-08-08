#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>


#include "SoulWorker.h"
#include "Setup.h"
#include "unistd.h" // For compiling in Windows MSVC

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


SoulWorker* player;
Maze* maze;

void loop() {
  Room* currRoom = player->room;

  printf("Hello, %s! Welcome to Cloudream!\n", (player->name));
  printf("You are a SoulWorker exploring the N-102 Lab in Candus City.\n");

  printf("You are in the entry room. %s\n", currRoom->info);

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
  loop();


  return 0;
}
