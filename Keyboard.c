#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "Keyboard.h"
#include "Error.h"
#include "SaveLoad.h"


#define CHAR_TO_INDEX(c) \
    ((c) == 'n' ? 0 : \
    (c) == 'e' ? 1 : \
    (c) == 's' ? 2 : \
    3)


typedef enum {
  MOVE_NORTH = 'n',
  MOVE_EAST = 'e',
  MOVE_SOUTH = 's',
  MOVE_WEST = 'w'
} Movement;

typedef enum {
  MOVEMENT,
  ACTIONS
} HELP_T;

/**
 * Valides the given exit direction. That is, whether the direction points to a closed exit.
 * @param dir The direction exit to validate
 * @param room The current room
 * @return True if valid, false otherwise
 */
static bool validExit(Movement dir, Room* room) {
  if ((dir == MOVE_NORTH && room->exits[0] == (void*) ((long long)NO_EXIT)) ||
      (dir == MOVE_EAST && room->exits[1] == (void*) ((long long)NO_EXIT)) ||
      (dir == MOVE_SOUTH && room->exits[2] == (void*) ((long long)NO_EXIT)) ||
      (dir == MOVE_WEST && room->exits[3] == (void*) ((long long)NO_EXIT))) {

    return false;
  }

  return true;
}

/**
 * Validates the given direction. That is, whether the direction is a direction at all.
 * @param dir The direction to validate
 * @param room The current room
 * @return True if valid, false otherwise
 */
static bool validMove(Movement* dir, Room* room) {
  if (*dir == MOVE_NORTH || *dir == MOVE_EAST || *dir == MOVE_SOUTH || *dir == MOVE_WEST) {
    while (!validExit(*dir, room)) {
      printf("That direction is closed. Try another direction! ");

      *dir = getchar();
      FLUSH()
      *dir = tolower(*dir);
    }

    // Fix

    return true;
  }
  
  return false;
}

static void displayHelp(HELP_T type) {
  if (type == MOVEMENT) {
    printf("Directions are north ('n'), east ('e'), south ('s'), and west ('w').");
  } else { // type == ACTIONS
    printf("Possible actions are:\n");
    printf("\t Open Inventory ('i')\n");
    printf("\t Move ('m')\n");
    printf("\t Save ('s')\n");
    printf("\t Save and Quit ('q')\n");
    printf("\t View self ('e')\n");
    printf("\t View gear ('g')\n");
    printf("\t Help message ('h')\n");
  }
}

/**
 * Saves te game and quits.
 */
static void quitGame() {
  saveGame();
  exit(0);
}

bool performAction(Commands action, SoulWorker* player) {
  if (action == WALK) {
    printf("What direction do you want to move? ");
    Movement dir = getchar();
    FLUSH()
    dir = tolower(dir);

    while(!validMove(&dir, player->room)) {
      printf("That is not a direction. Try again! For a list of acceptable direction, type 'h'. ");

      dir = getchar();
      FLUSH()
      dir = tolower(dir);

      if (dir == 'h') displayHelp(MOVEMENT);
    }

    printf("Entering room...\n");

    player->room = player->room->exits[CHAR_TO_INDEX(dir)];

    printf("You are in %s...\n", player->room->info);

    if (player->room == (void*)(long long)(NO_EXIT)) handleError(ERR_MEM, FATAL, "Cannot access exit!\n");

    // printf("You are in %s...\n", currRoom->info);
  } else if (action == OPEN_INVENTORY) viewInventory(player);
    else if (action == HELP) displayHelp(ACTIONS);
    else if (action == SAVE) saveGame();
    else if (action == QUIT) quitGame();
    else if (action == INFO) viewSelf(player);
    else if (action == GEAR) viewGear(player);
    else return false;

  return true;
}