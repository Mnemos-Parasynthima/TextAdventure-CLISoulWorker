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
typedef struct Item {               // 3B+1B(PAD) = 4B
  item_t _item; // The type of item                 1B
  unsigned short count; // The amount of that item  2B
} Item;

// The player model.
typedef struct SoulWorker {               // 234B+6B(PAD) = 240B
  char* name; // The name of the player                       8B
  Room* room; // The current room that the player is in       8B
  unsigned int xp; // The current XP                          4B
  unsigned int hp; // The current HP                          4B
  unsigned int maxHP; // The max HP                           4B
  unsigned int dzenai; // The currency                        4B
  unsigned short invCount; // Current items in the inventory  2B
  Item inv[INV_CAP]; // The player's inventory     50B*4B = 200B  
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

/**
 * Deletes the structure and frees the memory for the end of the gametime.
 * @param sw The SoulWorker structure to delete and free
 */
void deleteSoulWorker(SoulWorker* sw);


#endif