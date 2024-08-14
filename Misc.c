#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Misc.h"
#include "Error.h"


/**
 * Compares the floats, with a provided epsilon for floating errors.
 * @param f1 Float 1
 * @param f2 Float 2
 * @return True if relativelt equal, false otherwise
 */
static bool floateq(float f1, float f2) {
  float EPS = 0.0001;

  return (fabs(f1 - f2) < EPS);
}

/**
 * Whether the two SoulWeapons are equal in all fields, except durability and upgrades.
 * @param sw1 SoulWeapon 1
 * @param sw2 SoulWeapon 2
 * @return True if equal, false otherwise
 */
static bool equalSoulWeapon(SoulWeapon* sw1, SoulWeapon* sw2) {
  return ((sw1->acc == sw2->acc) &&
          (sw1->atk == sw2->atk) &&
          (floateq(sw1->atk_crit, sw2->atk_crit)) &&
          (sw1->atk_crit_dmg == sw2->atk_crit_dmg) &&
          (sw1->lvl == sw2->lvl) &&
          (strcmp(sw1->name, sw2->name) == 0));
}

/**
 * Whether the two pieces of armor are equal in all fields.
 * @param arm1 Armor 1
 * @param arm2 Armor 2
 * @return True if equal, false otherwise
 */
static bool equalArmor(Armor* arm1, Armor* arm2) {
  return ((arm1->type == arm2->type) &&
          (arm1->acc == arm2->acc) &&
          (arm1->def == arm2->def) &&
          (arm1->lvl == arm2->lvl) &&
          (strcmp(arm1->name, arm2->name) == 0));
}

/**
 * Whether the two HP kits are equal in all fields, except description.
 * @param kit1 HP Kit 1
 * @param kit2 HP Kit 2
 * @return True if equal, false otherwise
 */
static bool equalHPKits(HPKit* kit1, HPKit* kit2) {
  return kit1->type == kit2->type;
}

/**
 * Whether the two upgrade materials are equal in all fields, except description.
 * @param up1 Upgrade material 1
 * @param up2 Upgrade material 2
 * @return True if equal, false otherwise
 */
static bool equalUpgrade(Upgrade* up1, Upgrade* up2) {
  return (up1->rank == up2->rank && up1->type == up2->type);
}

bool equalItems(Item* item1, Item* item2) {
  if (item1 == NULL || item2 == NULL) return false;

  if (item1->type != item2->type) return false;

  // For each specific item type, check data equality
  switch (item1->type) {
    case SOULWEAPON_T:
      return equalSoulWeapon((SoulWeapon*)item1->_item, (SoulWeapon*)item2->_item);
      break;
    case HELMET_T:
    case SHOULDER_GUARD_T:
    case CHESTPLATE_T:
    case BOOTS_T:
      return equalArmor((Armor*)item1->_item, (Armor*)item2->_item);
      break;
    case HP_KITS_T:
      return equalHPKits((HPKit*)item1->_item, (HPKit*)item2->_item);
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
    case ARMOR_UPGRADE_MATERIALS_T:
      return equalUpgrade((Upgrade*)item1->_item, (Upgrade*)item2->_item);
      break;
    case SLIME_T:
      return true;
      break;
    default:
      return false;
      break;
  }

  return false;
}

/**
 * Creates the name for the given HP kit.
 * @param hpKit The HP kit to get the name
 * @return The name
 */
static char* getHPKitName(HPKit* hpKit) {
  // "**** HP Kit"
  char* name = (char*) malloc(12);
  if (name == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for HP Kit name!\n");

  char* type;
  switch (hpKit->type) {
    case DEKA:
      type = "Deka";
      break;
    case MEGA:
      type = "Mega";
      break;
    case PETA:
      type = "Peta";
      break;
    default:
      type = "ERR_";
      break;
  }

  sprintf(name, "%s HP Kit", type);  

  return name;
}

/**
 * Creates the name for the given upgrade material.
 * @param upgrade The upgrade material
 * @return The name
 */
static char* getUpgradeName(Upgrade* upgrade) {
  // "* Weapon|Armor Upgrade Material"  
  char* name = (char*) malloc(26);
  if (name == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for upgrade material name!\n");

  char* type;
  switch (upgrade->type) {
    case WEAPON:
      type = "Weapon";
      break;
    case ARMOR:
      type = "Armor";
      break;
    default:
      type = "ERROR";
      break;
  }

  sprintf(name, "%c %s Upgrade Material", (char)upgrade->rank, type);

  return name;
}

char *getItemName(Item *item) {
  char* name;

  switch (item->type) {
    case SOULWEAPON_T:
      SoulWeapon* sw_t = (SoulWeapon*) item->_item;
      name = sw_t->name;
      break;
    case HELMET_T:
      Armor* helm_t = (Armor*) item->_item;
      name = helm_t->name;
      break;
    case SHOULDER_GUARD_T:
      Armor* guard_t = (Armor*) item->_item;
      name = guard_t->name;
      break;
    case CHESTPLATE_T:
      Armor* plate_t = (Armor*) item->_item;
      name = plate_t->name;
      break;
    case BOOTS_T:
      Armor* boots_t = (Armor*) item->_item;
      name = boots_t->name;
      break;
    case HP_KITS_T:
      HPKit* hp_t = (HPKit*) item->_item;
      name = getHPKitName(hp_t);
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
      Upgrade* wu_t = (Upgrade*) item->_item;
      name = getUpgradeName(wu_t);
      break;
    case ARMOR_UPGRADE_MATERIALS_T:
      Upgrade* au_t = (Upgrade*) item->_item;
      name = getUpgradeName(au_t);
      break;
    case SLIME_T:
      name = "Slime";
      break;
    default:
      name = "NOT_AN_ITEM";
      break;
  }

  return name;
}

bool deleteSoulWeapon(SoulWeapon* sw) {
  free(sw->name);
  free(sw);
  sw = NULL;
}

bool deleteArmor(Armor* armor) {
  free(armor->name);
  free(armor);
  armor = NULL;
}

/**
 * Deletes items that only have the description allocated.
 * Note, even if the type is HPKit*, it does the same for the upgrade materials.
 * @param item Any other item
 */
static void deleteOther(HPKit* item) {
  free(item->desc);
  free(item);
  item = NULL;
}

bool deleteItem(Item* item) {
  if (item == NULL) return false;

  // Maybe turn to if statements
  switch (item->type) {
    case SOULWEAPON_T:
      deleteSoulWeapon((SoulWeapon*) item->_item);
    case HELMET_T:
    case SHOULDER_GUARD_T:
    case CHESTPLATE_T:
    case BOOTS_T:
      deleteArmor((Armor*) item->_item);
    case HP_KITS_T:
    case WEAPON_UPGRADE_MATERIALS_T:
    case ARMOR_UPGRADE_MATERIALS_T:
    case SLIME_T:
      deleteOther((HPKit*)item->_item);
  }

  free(item);
  item = NULL;

  return true;
}

bool deleteEnemy(Enemy* enemy) {
  if (enemy == NULL) return false;

  free(enemy->name);
  free(enemy->stats);
  free(enemy);
  enemy = NULL;

  return true;
}

bool deleteBoss(Boss* boss) {
  if (boss == NULL) return false;

  deleteEnemy(&(boss->base));
  // Make drop function that copies the structure (alloc 40B, transfer pointer ownership of gear and weapon)
  // deleteGear(boss->gear_drop);
  free(boss);
  boss = NULL;

  return true;
}