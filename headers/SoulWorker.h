#ifndef _SOULWORKER_H
#define _SOULWORKER_H

#include <stdbool.h>

#include "Setup.h"
// #include "Bitmap.h"


#define INV_CAP 25
#define ITEM_MAX 99
#define EQUIPPED_SKILL_COUNT 5
#define TOTAL_SKILLS 10

// The player model.
typedef struct SoulWorker {             // 494B+2B(PAD) = 496B
  str name; // The name of the player                       8B
  Room* room; // The current room that the player is in     8B
  uint xp; // The current XP                                4B
  uint lvl; // The current level                            4B
  uint hp; // The current HP                                4B
  uint maxHP; // The max HP                                 4B
  uint dzenai; // The currency                              4B
  Gear gear; //                                            40B
  Stats* stats; //                                          8B
  struct SkillTree* skills; //                              8B
  ushort invCount; // Current items in the inventory        2B
  Item inv[INV_CAP]; // The player's inventory  25B*16B = 400B
} SoulWorker;

// The player skill tree
typedef struct SkillTree {             // 366B+2B(PAD) = 368B
  Skill* equippedSkills[EQUIPPED_SKILL_COUNT];  // 8B*5 = 40B
  Skill skills[TOTAL_SKILLS];                // 32B*10 = 320B
                                                        // 2B
  ushort skillStatus; // Whether a skill is unlocked or not, is a bitmap; bit-0 is skills[0], bit-n is skills[n] where n is TOTAL_SKILLS
  uint totalSkillPoints; // How many points the player has 4B
} SkillTree;

/**
 * Initializes the player model with the given name.
 * @param name The name of the player
 * @return The SoulWorker player model
 */
SoulWorker* initSoulWorker(str name);

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
 * Displays player info, including the equipped gear.
 * @param sw The player
 */
void viewSelf(SoulWorker* sw);

/**
 * Displays player skills, including currently equipped.
 * @param sw The player
 */
void viewSkills(SoulWorker* sw);

/**
 * Sets the given skill at the provided slot of the player
 * @param sw The player's skill tree
 * @param skill The skill to set
 * @param slot The slot to set the skill at
 */
void setSkill(SkillTree* skillTree, Skill* skill, uint slot);

/**
 * Whether the given skill number (its ID) points to an unlocked skill
 * @param skillTree The player's skill tree
 * @param skillNum The skill ID to check
 * @return True if the skill has been unlock, false otherwise
 */
bool isSkillUnlocked(SkillTree* skillTree, uint skillNum);

/**
 * Unlocks the given skill using its ID
 * @param skillTree The player's skill tree
 * @param skillNum The ID of the skill to unlock
 */
void skillUnlock(SkillTree* skillTree, uint skillNum);

/**
 * Upgrades the given skill.
 * @param skill The skill to upgrade
 */
void upgradeSkill(Skill* skill);

/**
 * Unequips all the equipped gear.
 * @param sw The player
 */
void unequipGear(SoulWorker* sw);

/**
 * Equips the provided gear.
 * @param sw The player
 * @param item The gear
 */
void equipGear(SoulWorker* sw, Item* item);

/**
 * Updates the player's XP, increase the level if necessary.
 * @param sw The player
 * @param xp The amount to increase XP by
 */
void updateXP(SoulWorker* sw, uint xp);

/**
 * Deletes the structure and frees the memory for the end of the gametime.
 * @param sw The SoulWorker structure to delete and free
 */
void deleteSoulWorker(SoulWorker* sw);


#endif