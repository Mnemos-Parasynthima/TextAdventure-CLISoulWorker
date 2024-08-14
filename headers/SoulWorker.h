#ifndef _SOULWORKER_H
#define _SOULWORKER_H

#include <stdbool.h>

#include "Maze.h"
#include "Setup.h"
#include "Misc.h"


#define INV_CAP 25
#define ITEM_MAX 99

// The player model.
typedef struct SoulWorker {             // 486B+2B(PAD) = 488B
  char* name; // The name of the player                     8B
  Room* room; // The current room that the player is in     8B
  uint xp; // The current XP                                4B
  uint lvl; // The current level                            4B
  uint hp; // The current HP                                4B
  uint maxHP; // The max HP                                 4B
  uint dzenai; // The currency                              4B
  struct gear { //                                         40B
    SoulWeapon* sw; // 8B
    Armor* helmet; // 8B
    Armor* guard; // 8B
    Armor* chestplate; // 8B
    Armor* boots; // 8B
  } gear;
  Stats* stats; //                                          8B
  ushort invCount; // Current items in the inventory        2B
  Item inv[INV_CAP]; // The player's inventory  25B*16B = 400B
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
bool addToInv(SoulWorker* sw, Item* loot);

/**
 * Removes a given loot from a player.
 * @param sw The target player
 * @param loot The loot item
 * @param count How many to remove
 * @return True if it was removed, false otherwise
 */
bool removeFromInv(SoulWorker* sw, Item* loot, ushort count);

/**
 * Displays the current inventory of the player.
 * @param sw The player
 */
void viewInventory(SoulWorker* sw);

/**
 * Displays player info.
 * @param sw The player
 */
void viewSelf(SoulWorker* sw);

/**
 * Displays the equipped gear (armor and weapon) of the player.
 * @param sw The player
 */
void viewGear(SoulWorker* sw);

/**
 * Deletes the structure and frees the memory for the end of the gametime.
 * @param sw The SoulWorker structure to delete and free
 */
void deleteSoulWorker(SoulWorker* sw);


#endif