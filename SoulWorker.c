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
  sw->stats = NULL;

  sw->gear.sw = NULL;
  sw->gear.helmet = NULL;
  sw->gear.guard = NULL;
  sw->gear.chestplate = NULL;
  sw->gear.boots = NULL;
  

  for(int i = 0; i < INV_CAP; i++) {
    sw->inv[i]._item = NO_ITEM;
    sw->inv[i].count = 0;
    sw->inv[i].type = NONE;
  }

  return sw;
}


bool addToInv(SoulWorker* sw, Item* loot) {
  // Adding to inventory basically means transfer of pointer possession
  // Aka room a holds an Item, it will no longer hold the pointer to the object Item
  // The player will now hold it
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

    // Enough space, add it
    for (int i = 0; i < INV_CAP; i++) {
      // Finding first slot with no item
      if (sw->inv[i].type == NONE) {
        sw->inv[i]._item = item->_item;
        sw->inv[i].count = item->count;
        sw->inv[i].type = item->type;
        sw->invCount++;

        printf("Item has been added to the inventory!\n");
        return true;
      }
    }
  }

  // Gear are non-stackable
  // However, because each gear is different, it will get caught by the first equalItems check
  // So it then is guaranteed that the gear is different, thus it is handle by the item == NULL block

  for (int i = 0; i < INV_CAP; i++) {
    if (equalItems(&(sw->inv[i]), loot)) {
      sw->inv[i].count++;

      printf("Item count has been increased!");
      return true;
    }
  }

  return false;

  // Make sure that the prior owner of the loot pointer no longer holds the pointer
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

  if (item != NULL) {
    printf("You do not have that item in your inventory!\n");

    return false;
  }

  for (int i = 0; i < INV_CAP; i++) {
    if (equalItems(&(sw->inv[i]), loot)) {
      if (sw->inv[i].count > 1) {
        sw->inv[i].count -= count; // Handle proper count user input at caller fxn
      } else {
        free(sw->inv[i]._item);
        sw->inv[i]._item = NO_ITEM;
        sw->inv[i].count = 0;
        sw->inv[i].type = NONE;
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
        char* name = getItemName(sw->inv[i]._item);

        printf("%s * %d\n", name, sw->inv[i].count);

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

void viewSelf(SoulWorker* sw) {
  /**
   * [name], LVL [lvl]; [hp]/[maxHP]\n
   * XP: [xp]; [dzenai] DZ\n
   * ATK: [atk]; DEF: [def]; ACC: [acc]; ATK CRIT DMG: [atk_crit_dmg]; ATK CRIT: [atk_crit]\n
   */

  printf("%s, LVL %d; %d/%d\nXP: %d; %d DZ\nATK: %d; DEF: %d; ACC: %d; ATK CRIT DMG: %d; ATK CRIT: %3.2f\n", 
          sw->name, sw->lvl, sw->hp, sw->maxHP, sw->xp, sw->dzenai,
          sw->stats->ATK, sw->stats->DEF, sw->stats->ACC, sw->stats->ATK_CRIT_DMG, sw->stats->ATK_CRIT);
}

void viewGear(SoulWorker* sw) {
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

  char* weaponStats, helmetStats, shoulderGuardStats, chestplateStats, bootsStats;
  int weaponNameLen, helmetNameLen, shoulderGuardNameLen, chestplateNameLen, bootsNameLen;
  weaponNameLen = strlen(swp->name);
  helmetNameLen = strlen(helmet->name);
  shoulderGuardNameLen = strlen(guard->name);
  chestplateNameLen = strlen(chestplate->name);
  bootsNameLen = strlen(boots->name);

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
  else sprintf(weaponStats, "%s, ATK: %d, ACC: %d, ATK CRIT: %3.2f, ATK CRIT DMG: %d,\n\tLVL: %d, UPGRADES: %d, DURABILITY, %d", 
                            swp->name, swp->atk, swp->acc, swp->atk_crit, swp->atk_crit_dmg, swp->lvl, swp->upgrades, swp->durability);

  if (helmet == NO_ITEM) sprintf(helmetStats, "Unequipped");
  else sprintf(helmetStats, "%s; LVL: %d, ACC: %d, DEF: %d", helmet->name, helmet->lvl, helmet->acc, helmet->def);

  if (guard == NO_ITEM) sprintf(shoulderGuardStats, "Unequipped");
  else sprintf(shoulderGuardStats, "%s; LVL: %d, ACC: %d, DEF: %d", guard->name, guard->lvl, guard->acc, guard->def);

  if (chestplate == NO_ITEM) sprintf(chestplateStats, "Unequipped");
  else sprintf(chestplateStats, "%s; LVL: %d, ACC: %d, DEF: %d", chestplate->name, chestplate->lvl, chestplate->acc, chestplate->def);

  if (boots == NO_ITEM) sprintf(bootsStats, "Unequipped");
  else sprintf(bootsStats, "%s; LVL: %d, ACC: %d, DEF: %d", boots->name, boots->lvl, boots->acc, boots->def);

  printf("\tSoulWeapon: %s\n", weaponStats);
  printf("\tHelmet: %s\n", helmetStats);
  printf("\tShoulder Guard: %s\n", shoulderGuardStats);
  printf("\tChestplate: %s\n", chestplateStats);
  printf("\tBoots: %s\n", bootsStats);
}

void deleteSoulWorker(SoulWorker* sw) {
  if (sw == NULL) return;

  free(sw->name);
  free(sw->stats);
  // room will be taken care of by Maze cleanup
  deleteSoulWeapon(sw->gear.sw);
  deleteArmor(sw->gear.helmet);
  deleteArmor(sw->gear.guard);
  deleteArmor(sw->gear.chestplate);
  deleteArmor(sw->gear.boots);
  free(sw);
  sw = NULL;
}