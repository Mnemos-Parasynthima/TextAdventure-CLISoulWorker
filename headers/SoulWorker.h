#ifndef _SOULWORKER_H
#define _SOULWORKER_H

#include <stdbool.h>

#include "Maze.h"
#include "Setup.h"


#define INV_CAP 50
#define ITEM_MAX 99

typedef enum {
  SLIME = 's',
  KEY = 'k'
} item_t;

// The Item model.
typedef struct Item {
  item_t _item; // The type of item
  unsigned short count; // The amount of that item
} Item;

// The player model.
typedef struct SoulWorker {
  char* name; // The name of the player
  unsigned int xp; // The current XP
  unsigned int hp; // The current HP
  unsigned int maxHP; // The max HP
  Item inv[INV_CAP]; // The player's inventory
  unsigned short invCount; // Current items in the inventory
  unsigned int dzenai; // The currency
  Room* room; // The current room that the player is in
} SoulWorker;

/**
 * Initializes the player model with the given name.
 * @param name The name of the player
 * @return The SoulWorker player model
 */
SoulWorker* initSoulWorker(char* name);

/**
 * Adds a given loot item to a player.
 * @param sw The target player
 * @param loot The loot item
 * @return True if it was added, false otherwise
 */
bool addToInv(SoulWorker* sw, item_t loot);

/**
 * Removes a given loot from a player.
 * @param sw The target player
 * @param loot The loot item
 * @return True if it was removed, false otherwise
 */
bool removeFromInv(SoulWorker* sw, item_t loot);

/**
 * Displays the current inventory of the player
 * @param sw The player
 */
void viewInventory(SoulWorker* sw);


#endif