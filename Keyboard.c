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

#define FLUSH_


typedef enum {
  MOVE_NORTH = 'n',
  MOVE_EAST = 'e',
  MOVE_SOUTH = 's',
  MOVE_WEST = 'w'
} Movement;


typedef enum {
  INV_SELL = 's', // Sell item; s
  INV_INFO = 'i', // View item info; i
  INV_QUIT = 'q', // Exit inventory; q
  INV_HELP = 'h'
} Inventory;

typedef enum {
  MOVEMENT,
  ACTIONS,
  INVENTORY
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


static void sellItem(Item* item, ushort count) {
  // count is handled by caller

  uint dz = 0;

  switch (item->type) {
    case SOULWEAPON_T:
    case HELMET_T:
    case SHOULDER_GUARD_T:
    case CHESTPLATE_T:
    case BOOTS_T:
      // Make dz be a range depending on lvl
      dz = 30;
      break;
    case HP_KITS_T:
      // Make dz depend on type
      dz = 50;
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
    case ARMOR_UPGRADE_MATERIALS_T:
      // Make dz depend on rank
      dz = 300;
      break;
    case SLIME_T:
      dz = 100;
    default:
      break;
  }

  uint total = dz * count;
  player->dzenai += total;
  printf("%d %s has been sold for %d dzenai!\n", count, getItemName(item), total);
  removeFromInv(player, item, count);
}

static Item* validItem(uchar itemI) {
  if (itemI < 1 || itemI > INV_CAP) return NULL;
  if (player->inv[itemI-1]._item == NULL) return NULL;

  return &(player->inv[itemI-1]);
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

static bool validInvAction(Inventory* inv) {
  return (*inv == INV_SELL || *inv == INV_INFO || *inv == INV_HELP || *inv == INV_QUIT);
}

static void displayHelp(HELP_T type) {
  if (type == MOVEMENT) {
    printf("Directions are north ('n'), east ('e'), south ('s'), and west ('w').");
  } else if (type == INVENTORY) {
    printf("Possible actions are:\n");
    printf("\t Sell item ('s')\n");
    printf("\t View item info ('i')\n");
    printf("\t Close inventory ('q')\n");
    printf("\t Help message ('h')\n");
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
  } else if (action == OPEN_INVENTORY) {
    viewInventory(player);

    printf("Inventory: What do you want to do? ");
    Inventory inv = getchar();
    FLUSH()
    inv = tolower(inv);

    while (inv != INV_QUIT) {
      while(!validInvAction(&inv)) {
        printf("That is not a valid action. Try again! For a list of acceptable actions, type 'h'. ");

        inv = getchar();
        FLUSH()
        inv = tolower(inv);
      }

      if (inv == INV_SELL) {
        printf("What item? Use a number for its position. ");
        uchar _item = getchar();
        FLUSH()
        uchar itemI = _item - '0';

        Item* item = validItem(itemI);
        while (item == NULL) {
          printf("That is not a valid inventory entry!\n Try again! ");
          _item = getchar();
          FLUSH()
          uchar itemI = _item - '0';

          item = validItem(itemI);
        }

        printf("How many do you want to sell? ");
        uchar _count = getchar();
        FLUSH()
        uchar count = _count - '0';

        while (count < 0 || count > item->count) {
          printf("Invalid count! Try again! ");
          _count = getchar();
          FLUSH()
          count = _count - '0';
        }        

        sellItem(item, count);
      } else if (inv == INV_INFO) {
        printf("View item info\n");
      } else if (inv == INV_HELP) {
        displayHelp(INVENTORY);
      } else if (inv == INV_QUIT) {
        printf("Closing inventory\n");
        break;
      }

      printf("Inventory: What do you want to do? ");
      inv = getchar();
      FLUSH()
      inv = tolower(inv);
    } 
  } else if (action == HELP) displayHelp(ACTIONS);
    else if (action == SAVE) saveGame();
    else if (action == QUIT) quitGame();
    else if (action == INFO) viewSelf(player);
    else if (action == GEAR) viewGear(player);
    else return false;

  return true;
}