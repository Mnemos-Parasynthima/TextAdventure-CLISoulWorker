#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "Setup.h"

#ifndef _SOULWORKER_
#define _SOULWORKER_

#define INV_CAP 50

// The player model.
typedef struct SoulWorker {
  char* name; // The name of the player
  int xp; // The current XP
  int hp; // The current HP
  int maxHP; // The max HP
  char inv[INV_CAP]; // The player's inventory
  int invCount; // Current items in the inventory
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
bool addToInv(SoulWorker* sw, char loot);

/**
 * Removes a given loot from a player.
 * @param sw The target player
 * @param loot The loot item
 * @return True if it was removed, false otherwise
 */
bool removeFromInv(SoulWorker* sw, char loot);

/**
 * Displays the current inventory of the player
 * @param sw The player
 */
void viewInventory(SoulWorker* sw);


#endif