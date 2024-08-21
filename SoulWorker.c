#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SoulWorker.h"
#include "Error.h"

#define NO_ITEM NULL

SoulWorker* initSoulWorker(char* name) {
  SoulWorker* sw = (SoulWorker*) malloc(sizeof(SoulWorker));
  if (sw == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for the player!\n");  

  sw->name = name;
  sw->hp = 100;
  sw->xp = 0;
  sw->lvl = 0;
  sw->invCount = 0;
  sw->dzenai = 0;
  sw->maxHP = 100;

  sw->room = NULL;

  sw->gear.sw = NO_ITEM;
  sw->gear.helmet = NO_ITEM;
  sw->gear.guard = NO_ITEM;
  sw->gear.chestplate = NO_ITEM;
  sw->gear.boots = NO_ITEM;
  

  for(int i = 0; i < INV_CAP; i++) {
    sw->inv[i]._item = NO_ITEM;
    sw->inv[i].count = 0;
    sw->inv[i].type = NONE;
  }

  Stats* stats = (Stats*) malloc(sizeof(Stats));
  if (stats == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for player stats!\n");

  stats->ACC = 100;
  stats->ATK = 100;
  stats->ATK_CRIT = 5.0;
  stats->ATK_CRIT_DMG = 10;
  stats->DEF = 100;
  sw->stats = stats;

  return sw;
}


bool addToInv(SoulWorker* sw, Item* loot) {
  /**
   * When rooms are setup and items created, it creates an Item structure for that room
   * However, when that item is picked up, the malloc'd struct is kept but the Item struct in
   *  the inventory is filled with its data, especially a copy of the void*. However, that Item struct is never freed.
   * When the player is deleted, it only frees its built-in Item structs and the structs of its void*
   *  but never the original malloc'd Item structure, thus having a memory leak.
   * Doing deleteItem when adding to inventory frees its void*, which is not what is intended.
   */

  Item* item = NULL;

  // Look for item
  for (int i = 0; i < INV_CAP; i++) {
    if (equalItems(&(sw->inv[i]), loot)) {
      item = &(sw->inv[i]);
      break;
    }
  }

  if (item == NULL) { // Item not in inventory
    // Make sure enough space
    if (sw->invCount == INV_CAP) {
      printf("Your inventory is full! You must remove some items!\n");

      return false;
    }

    // Gear are non-stackable
    // However, because each gear is different, it will get caught by the first equalItems check
    // So it then is guaranteed that the gear is different, thus it is handle by the item == NULL block

    // Enough space, add it
    for (int i = 0; i < INV_CAP; i++) {
      // Finding first slot with no item
      if (sw->inv[i].type == NONE) {
        sw->inv[i]._item = loot->_item;
        sw->inv[i].count = loot->count;
        sw->inv[i].type = loot->type;
        sw->invCount++;

        printf("Item has been added to the inventory!\n");
        return true;
      } else if (equalItems(&(sw->inv[i]), loot)) { // Updating existing item
        sw->inv[i].count++;

        printf("Item count has been increased!");
        return true;

        /**
         * The item in the map does not get removed. When an existing item is picked up,
         * only the inv item count is increased, then the Item structure if freed, but not its void* one
         * leading to a memory leak.
         * Thus, Item._item must be freed in this case
         */
        switch (sw->inv[i].type) {
          case SOULWEAPON_T:
            deleteSoulWeapon((SoulWeapon*) sw->inv[i]._item);
            break;
          case HELMET_T:
          case SHOULDER_GUARD_T:
          case CHESTPLATE_T:
          case BOOTS_T:
            deleteArmor((Armor*) sw->inv[i]._item);
            break;
          case HP_KITS_T:
          case WEAPON_UPGRADE_MATERIALS_T:
          case ARMOR_UPGRADE_MATERIALS_T:
          case SLIME_T:
            deleteOther((HPKit*) sw->inv[i]._item);
            break;
          default:
            break;
        }
        sw->inv[i]._item = NO_ITEM;
      }
    }
  }

  return false;
}

bool removeFromInv(SoulWorker* sw, Item* loot, ushort count) {
  // Note, Item* loot is temporary until a way to get player input is decided

  Item* item = NULL;

  // Look for item
  for (int i = 0; i < INV_CAP; i++) {
    if (equalItems(&(sw->inv[i]), loot)) {
      item = &(sw->inv[i]);
      break;
    }
  }

  if (item == NULL) {
    printf("You do not have that item in your inventory!\n");

    return false;
  }

  for (int i = 0; i < INV_CAP; i++) {
    if (equalItems(&(sw->inv[i]), loot)) {
      sw->inv[i].count -= count;

      if (sw->inv[i].count == 0) {
        // Not using deleteItem() because Item struct is built-in the inv
        // not as a pointer
        // And deleteItem() frees and nulls the Item struct
        // Need to free and null the actual item itself (void*)

        void* _item = sw->inv[i]._item;

        switch (sw->inv[i].type) {
          case SOULWEAPON_T:
            deleteSoulWeapon((SoulWeapon*) _item);
            break;
          case HELMET_T:
          case SHOULDER_GUARD_T:
          case CHESTPLATE_T:
          case BOOTS_T:
            deleteArmor((Armor*) _item);
            break;
          case HP_KITS_T:
          case WEAPON_UPGRADE_MATERIALS_T:
          case ARMOR_UPGRADE_MATERIALS_T:
          case SLIME_T:
            deleteOther((HPKit*) _item);
            break;
          default:
            break;
        }

        sw->inv[i]._item = NO_ITEM;
        sw->inv[i].count = 0;
        sw->inv[i].type = NONE;

        sw->invCount--;
      }

      return true;
    }
  }

  return false;
}

void viewInventory(SoulWorker* sw) {
  if (sw->invCount == 0) printf("You have no items.\n");
  else {
    printf("You have %d item%s: \n", sw->invCount, (sw->invCount == 1) ? "" : "s");

    for (int i = 0; i < INV_CAP; i++) {
      if (sw->inv[i]._item != NO_ITEM) {
        char* name = getItemName(&(sw->inv[i]));

        printf("%d: %s * %d\n", i+1, name, sw->inv[i].count);

        // Since some strings have been alloc'd, free them
        switch (sw->inv[i].type) {
          case HP_KITS_T:
          case WEAPON_UPGRADE_MATERIALS_T:
          case ARMOR_UPGRADE_MATERIALS_T:
            free(name);
            name = NULL;
          default:
            break;
        }
      }
    }
  }
}

/**
 * Displays the equipped gear (armor and weapon) of the player.
 * @param sw The player
 */
static void viewGear(SoulWorker* sw) {
  SoulWeapon* swp = sw->gear.sw;
  Armor* helmet = sw->gear.helmet;
  Armor* guard = sw->gear.guard;
  Armor* chestplate = sw->gear.chestplate;
  Armor* boots = sw->gear.boots;
  /**
   * [weapon]: (Unequipped)|([name], ATK: [atk], ACC: [acc], ATK CRIT: [atk_crit], ATK CRIT DMG: [atk_crit_dmg],\n
   * \tLVL: [lvl], UPGRADES: [upgrades], DURABILITY: [durability])
   * [armor piece]: (Unequipped)|([name]; LVL: [lvl], ACC: [acc], DEF: [def])
   */

  char *weaponStats, *helmetStats, *shoulderGuardStats, *chestplateStats, *bootsStats;
  int weaponNameLen, helmetNameLen, shoulderGuardNameLen, chestplateNameLen, bootsNameLen;
  if (swp != NO_ITEM) weaponNameLen = strlen(swp->name);
  if (helmet != NO_ITEM) helmetNameLen = strlen(helmet->name);
  if (guard != NO_ITEM) shoulderGuardNameLen = strlen(guard->name);
  if (chestplate != NO_ITEM) chestplateNameLen = strlen(chestplate->name);
  if (boots != NO_ITEM) bootsNameLen = strlen(boots->name);

  // If no gear piece equipped, only get space for "Unequipped\0"
  // Otherwise, space for name and its stats
  //                                                            [name] + rest + [lvl] + [acc] + [def] + \0
  helmetStats = (char*)malloc((helmet == NO_ITEM) ? 11 : (helmetNameLen + 21 + 3 + 5 + 3 + 1));
  shoulderGuardStats = (char*)malloc((guard == NO_ITEM) ? 11 : (shoulderGuardNameLen + 21 + 3 + 5 + 3 + 1));
  chestplateStats = (char*)malloc((chestplate == NO_ITEM) ? 11 : (chestplateNameLen + 21 + 3 + 5 + 3 + 1));
  bootsStats = (char*)malloc((boots == NO_ITEM) ? 11 : (bootsNameLen + 21 + 3 + 5 + 3 + 1));

  weaponStats = (char*)malloc((swp == NO_ITEM) ? 11 : (weaponNameLen + 75 + 5 + 5 + 6 + 5 + 3 + 3 + 3 + 1));
  // atk_crit: ###.##

  if (swp == NO_ITEM) sprintf(weaponStats, "Unequipped");
  else sprintf(weaponStats, "%s, ATK: %d, ACC: %d, ATK CRIT: %3.2f, ATK CRIT DMG: %d,\n\t\tLVL: %d, UPGRADES: %d, DURABILITY, %d", 
                            swp->name, swp->atk, swp->acc, swp->atk_crit, swp->atk_crit_dmg, swp->lvl, swp->upgrades, swp->durability);

  if (helmet == NO_ITEM) sprintf(helmetStats, "Unequipped");
  else sprintf(helmetStats, "%s; LVL: %d, ACC: %d, DEF: %d", helmet->name, helmet->lvl, helmet->acc, helmet->def);

  if (guard == NO_ITEM) sprintf(shoulderGuardStats, "Unequipped");
  else sprintf(shoulderGuardStats, "%s; LVL: %d, ACC: %d, DEF: %d", guard->name, guard->lvl, guard->acc, guard->def);

  if (chestplate == NO_ITEM) sprintf(chestplateStats, "Unequipped");
  else sprintf(chestplateStats, "%s; LVL: %d, ACC: %d, DEF: %d", chestplate->name, chestplate->lvl, chestplate->acc, chestplate->def);

  if (boots == NO_ITEM) sprintf(bootsStats, "Unequipped");
  else sprintf(bootsStats, "%s; LVL: %d, ACC: %d, DEF: %d", boots->name, boots->lvl, boots->acc, boots->def);

  printf("SoulWeapon: %s\n", weaponStats);
  printf("Helmet: %s\n", helmetStats);
  printf("Shoulder Guard: %s\n", shoulderGuardStats);
  printf("Chestplate: %s\n", chestplateStats);
  printf("Boots: %s\n", bootsStats);

  free(weaponStats);
  free(helmetStats);
  free(shoulderGuardStats);
  free(chestplateStats);
  free(bootsStats);
}

void viewSelf(SoulWorker* sw) {
  /**
   * [name], LVL [lvl]; [hp]/[maxHP]\n
   * XP: [xp]; [dzenai] DZ\n
   * ATK: [atk]; DEF: [def]; ACC: [acc]; ATK CRIT DMG: [atk_crit_dmg]; ATK CRIT: [atk_crit]\n
   */

  printf("%s, LVL %d; %d/%d\nXP: %d; %d DZ\nATK: %d; DEF: %d; ACC: %d; ATK CRIT DMG: %d; ATK CRIT: %3.2f\n\n", 
          sw->name, sw->lvl, sw->hp, sw->maxHP, sw->xp, sw->dzenai,
          sw->stats->ATK, sw->stats->DEF, sw->stats->ACC, sw->stats->ATK_CRIT_DMG, sw->stats->ATK_CRIT);
          
  viewGear(sw);
}

void unequipGear(SoulWorker *sw) {
  Item* gear = (Item*) malloc(sizeof(Item));
  gear->count = 1;

  Stats* stats = sw->stats;

  // Do not try unequipping when gear already unequipped
  if (sw->gear.sw != NO_ITEM) {
    gear->type = SOULWEAPON_T;
    gear->_item = sw->gear.sw;
    if(!addToInv(sw, gear)) { free(gear); return; }

    stats->ATK -= sw->gear.sw->atk;
    stats->ACC -= sw->gear.sw->acc;
    stats->ATK_CRIT -= sw->gear.sw->atk_crit;
    stats->ATK_CRIT_DMG -= sw->gear.sw->atk_crit_dmg;
  }

  if (sw->gear.helmet != NO_ITEM) {
    gear->type = HELMET_T;
    gear->_item = sw->gear.helmet;
    if(!addToInv(sw, gear)) { free(gear); return; }

    stats->ACC -= sw->gear.helmet->acc;
    stats->DEF -= sw->gear.helmet->def;
  }

  if (sw->gear.guard != NO_ITEM) {
    gear->type = SHOULDER_GUARD_T;
    gear->_item = sw->gear.guard;
    if(!addToInv(sw, gear)) { free(gear); return; }

    stats->ACC -= sw->gear.guard->acc;
    stats->DEF -= sw->gear.guard->def;
  }

  if (sw->gear.chestplate != NO_ITEM) {
    gear->type = CHESTPLATE_T;
    gear->_item = sw->gear.chestplate;
    if(!addToInv(sw, gear)) { free(gear); return; }

    stats->ACC -= sw->gear.chestplate->acc;
    stats->DEF -= sw->gear.chestplate->def;
  }

  if (sw->gear.boots != NO_ITEM) {
    gear->type = BOOTS_T;
    gear->_item = sw->gear.boots;
    if(!addToInv(sw, gear)) { free(gear); return; }

    stats->ACC -= sw->gear.boots->acc;
    stats->DEF -= sw->gear.boots->def;
  }

  sw->gear.sw = NO_ITEM;
  sw->gear.helmet = NO_ITEM;
  sw->gear.guard = NO_ITEM;
  sw->gear.chestplate = NO_ITEM;
  sw->gear.boots = NO_ITEM;

  free(gear);
  gear = NULL;
}

void equipGear(SoulWorker *sw, Item *item) {
  Gear* gear = &(sw->gear);
  Stats* stats = sw->stats;
  
  // When equipping a gear piece that already is filled in the player's gear
  //  ie. equipping a SoulWeapon when there is one already equipped
  // simply swap the void* between both
  // But when equipping in a blank slot, transfer/copy the void* pointer from inv
  //  to gear.*, then "blanking" the inv slot (null out void* and 0 out type and count)

  void* temp = item->_item; // Hold the requested gear piece to equip

  switch (item->type) {
    case SOULWEAPON_T:
      if (gear->sw != NO_ITEM) { // One equipped already, swap
        // Remove current soulweapon effects
        stats->ATK -= gear->sw->atk;
        stats->ACC -= gear->sw->acc;
        stats->ATK_CRIT -= gear->sw->atk_crit;
        stats->ATK_CRIT_DMG -= gear->sw->atk_crit_dmg;

        item->_item = gear->sw; // Inv item slot will now hold equipped soulweapon
        gear->sw = temp; // Soulweapon gear slot will now hold requested inv item soulweapon
      } else { gear->sw = (SoulWeapon*) temp; temp = NULL; }

      stats->ATK += gear->sw->atk;
      stats->ACC += gear->sw->acc;
      stats->ATK_CRIT += gear->sw->atk_crit;
      stats->ATK_CRIT_DMG += gear->sw->atk_crit_dmg;
      break;
    case HELMET_T:
      if (gear->helmet != NO_ITEM) {
        item->_item = gear->helmet;
        gear->helmet = temp;

        stats->ACC -= gear->helmet->acc;
        stats->DEF -= gear->helmet->def;
      } else { gear->helmet = (Armor*) temp; temp = NULL; }

      stats->ACC += gear->helmet->acc;
      stats->DEF += gear->helmet->def;
      break;
    case SHOULDER_GUARD_T:
      if (gear->guard != NO_ITEM) {
        item->_item = gear->guard;
        gear->guard = temp;

        stats->ACC -= gear->helmet->acc;
        stats->DEF -= gear->helmet->def;
      } else { gear->guard = (Armor*) temp; temp = NULL; }

      stats->ACC += gear->helmet->acc;
      stats->DEF += gear->helmet->def;
      break;
    case CHESTPLATE_T:
      if (gear->chestplate != NO_ITEM) {
        item->_item = gear->chestplate;
        gear->chestplate = temp;

        stats->ACC -= gear->helmet->acc;
        stats->DEF -= gear->helmet->def;
      } else { gear->chestplate = (Armor*) temp; temp = NULL; }

      stats->ACC += gear->helmet->acc;
      stats->DEF += gear->helmet->def;
      break;
    case BOOTS_T:
      if (gear->boots != NO_ITEM) {
        item->_item = gear->boots;
        gear->boots = temp;

        stats->ACC -= gear->helmet->acc;
        stats->DEF -= gear->helmet->def;
      } else { gear->boots = (Armor*) temp; temp = NULL; }

      stats->ACC += gear->helmet->acc;
      stats->DEF += gear->helmet->def;
      break;
    default:
      break;
  }

  if (temp == NULL) { // NULL only when not swapping, meaning inv slot is to be empty
    item->_item = NO_ITEM;
    item->type = NONE;
    item->count = 0;
    sw->invCount--;
  }

  printf("Equipped!\n");
}

void deleteSoulWorker(SoulWorker *sw)
{
  if (sw == NULL) return;

  free(sw->name);
  free(sw->stats);
  // room will be taken care of by Maze cleanup
  deleteSoulWeapon(sw->gear.sw);
  deleteArmor(sw->gear.helmet);
  deleteArmor(sw->gear.guard);
  deleteArmor(sw->gear.chestplate);
  deleteArmor(sw->gear.boots);
  
  for (int i = 0; i < INV_CAP; i++) {
    void* _item = sw->inv[i]._item;

    switch (sw->inv[i].type) {
      case SOULWEAPON_T:
        deleteSoulWeapon((SoulWeapon*) _item);
        break;
      case HELMET_T:
      case SHOULDER_GUARD_T:
      case CHESTPLATE_T:
      case BOOTS_T:
        deleteArmor((Armor*) _item);
        break;
      case HP_KITS_T:
      case WEAPON_UPGRADE_MATERIALS_T:
      case ARMOR_UPGRADE_MATERIALS_T:
      case SLIME_T:
        deleteOther((HPKit*) _item);
        break;
      default:
        break;
    }
  }

  free(sw);
  sw = NULL;
}