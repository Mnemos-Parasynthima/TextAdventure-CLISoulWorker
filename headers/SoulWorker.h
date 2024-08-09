#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "Setup.h"

#ifndef _SOULWORKER_
#define _SOULWORKER_

#define INV_CAP 50
#define ITEM_MAX 99


typedef enum {
  SLIME = 's',
  KEY = 'k'
} item_t;


typedef struct Item {
  item_t _item;
  unsigned short count;
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