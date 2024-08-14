#ifndef _MISC_H
#define _MISC_H


#include <stdbool.h>


#define ushort unsigned short
#define uchar unsigned char
#define uint unsigned int


typedef struct Stats {                         // 12B
  ushort ATK; // The attack damage                 2B
  ushort DEF; // The defense                       2B
  ushort ACC; // The accuracy                      2B
  ushort ATK_CRIT_DMG; // The attack crit damage   2B
  float ATK_CRIT; // The attack crit percent       4B
} Stats;

// deal with -fshort-enums and pragma pack(2) later
// for now, enums are uints in gcc and msvc
typedef enum {
  NONE, // only when NO_ITEM
  SOULWEAPON_T,
  HELMET_T,
  SHOULDER_GUARD_T,
  CHESTPLATE_T,
  BOOTS_T,
  HP_KITS_T,
  WEAPON_UPGRADE_MATERIALS_T,
  ARMOR_UPGRADE_MATERIALS_T,
  SLIME_T
} item_t;
// The Item model.
typedef struct Item {      // 14B+2B(PAD) = 16B
  void* _item; // The item                   8B
  item_t type; // The type of the item       4B
  ushort count; // The amount of that item   2B
} Item;

typedef struct SoulWeapon {      // 21B+3B(PAD) = 24B
  char* name; // The name of the weapon            8B
  ushort atk; // The attack stat                   2B
  ushort acc; // The accuracy stat                 2B
  float atk_crit; // The attack crit percent       4B
  ushort atk_crit_dmg; // The attack crit damage   2B
  uchar lvl; // The weapon level                   1B
  uchar upgrades; // Upgrades done                 1B
  uchar durability; // Weapon durability           1B
} SoulWeapon;

typedef enum {
  HELMET,
  SHOULDER_GUARD,
  CHESTPLATE,
  BOOTS
} armor_t;
typedef struct Armor { //                16B
  char* name; // The armor name           8B
  armor_t type; // The type of armor      4B
  ushort acc; // The accuracy it provides 2B
  uchar def; // The defense stat          1B
  uchar lvl; // The armor level           1B
} Armor;

typedef enum {
  DEKA,
  MEGA,
  PETA
} hpkit_t;
typedef struct HPKit {
  hpkit_t type;
  // char type[8];
  char* desc;
} HPKit;

typedef enum {
  B = 'B', // Lvl 1-30
  A = 'A', // Lvl 30-60
  S = 'S' // Lvl 60-100
} value_t;
typedef enum {
  WEAPON,
  ARMOR
} upgrade_t;
typedef struct Upgrade {
  value_t rank; // The rank of the upgradebetter naming
  upgrade_t type; // Weapon or armor upgrade
  char* desc; // Description
} Upgrade;

typedef struct Slime {
  char* desc;
} Slime;


/**
 * Checks whether the two items are the exact same.
 * Note, there is a small chance that the items were generated with the exact same data but as seperate.
 * In that case, it will detect them as the same. This shall be intended behavior.
 * @param item1 Item 1
 * @param item2 Item 2
 * @return True if both are the same, false otherwise
 */
bool equalItems(Item* item1, Item* item2);

/**
 * Gets the specific name for the item, depending on its type.
 * @param item The item
 * @return The name
 */
char* getItemName(Item* item);

/**
 * Deletes the item, freeing the memory.
 * @param item The item to delete
 * @return True if the item was deleted, false otherwise
 */
bool deleteItem(Item* item);

/**
 * Deletes the given SoulWeapon, freeing memory.
 * @param sw The SoulWeapon to delete
 * @return True if the item was deleted, false otherwise
 */
bool deleteSoulWeapon(SoulWeapon* sw);

/**
 * Deletes the given armor piece, freeing memory.
 * @param armor The armor piece to delete
 * @return True if the item was deleted, false otherwise
 */
bool deleteArmor(Armor* armor);







typedef struct Enemy { // 25B+(7B) = 32B
  char* name; //                      8B
  uint xpPoints; //                   4B
  uint hp; //                         4B
  uchar lvl; //                       1B
  Stats* stats; //                    8B
} Enemy;

typedef struct Boss { // 72B
  Enemy base; //         32B
  struct gear { //       40B
    SoulWeapon* sw; //    8B
    Armor* helmet; //     8B
    Armor* guard; //      8B
    Armor* chestplate; // 8B
    Armor* boots; //      8B
  } gear_drop;
  // Note, the gear is only the drop, boss is not equipped
  // Add skills
} Boss;

/**
 * Deletes the enemy, freeing the memory.
 * @param enemy The enemy to delete
 * @return True if the enemy was deleted, false otherwise
 */
bool deleteEnemy(Enemy* enemy);

/**
 * Deletes the boss, freeing the memory.
 * @param boss 
 * @return 
 */
bool deleteBoss(Boss* boss);


















#endif