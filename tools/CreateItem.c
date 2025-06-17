#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Error.h"
#include "Colors.h"


#define FLUSH() do { int c; while ((c = getchar()) != '\n' && c != EOF); } while (0);

// This enum needs to match with Misc.h enum!!!!
typedef enum {
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

/**
 * Converts the given item number to its string representation.
 * @param item The item type
 * @return The representing string
 */
static str itemToString(item_t item) {
  str ret = NULL;

  switch (item) {
    case SOULWEAPON_T:
      ret = "SoulWeapon";
      break;
    case HELMET_T:
      ret = "Helmet";
      break;
    case SHOULDER_GUARD_T:
      ret = "Shoulder Guard";
      break;
    case CHESTPLATE_T:
      ret = "Chestplate";
      break;
    case BOOTS_T:
      ret = "Boots";
      break;
    case HP_KITS_T:
      ret = "HP Kit";
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
      ret = "Weapon Upgrade";
      break;
    case ARMOR_UPGRADE_MATERIALS_T:
      ret = "Armor Upgrade";
      break;
    case SLIME_T:
      ret = "Slime";
      break;
    default:
      ret = "UNKNOWN";
      break;
  }

  return ret;
}

/**
 * Creates soulweapons of num items, writing them to the appropriate file.
 * @param file The soulweapon.item file
 * @param num How many weapons to create
 */
static void createSoulWeapon(FILE* file, int num) {
  int id = 0;

  str line = NULL;
  size_t n;

  int stat;
  float statf;

  while (id != num) {
    printf("For SoulWeapon %d:\n", id);

    fprintf(file, "%d\n", id);
    printf("\t%sSet SoulWeapon id to %d\n%s", GREEN, id, RESET);

    fputs("1\n", file);
    printf("\t%sSet count to 1\n%s", GREEN, RESET);

    printf("\tEnter name: ");
    getline(&line, &n, stdin);
    fputs(line, file);
    printf("\t%sSet SoulWeapon name to %s%s", GREEN, line, RESET);

    printf("\tEnter the attack: ");
    fscanf(stdin, "%d", &stat);
    fprintf(file, "%d\n", stat);
    printf("\t%sSet attack stat to %d%s\n", GREEN, stat, RESET);

    printf("\tEnter the accuracy: ");
    fscanf(stdin, "%d", &stat);
    fprintf(file, "%d\n", stat);
    printf("\t%sSet accuracy stat to %d%s\n", GREEN, stat, RESET);

    // Check floating for other
    printf("\tEnter the attack crit: ");
    fscanf(stdin, "%f", &statf);
    fprintf(file, "%3.2f\n", statf);
    printf("\t%sSet attack crit stat to %3.2f%s\n", GREEN, statf, RESET);

    printf("\tEnter the attack crit damage: ");
    fscanf(stdin, "%d", &stat);
    fprintf(file, "%d\n", stat);
    printf("\t%sSet attack crit stat to %d%s\n", GREEN, stat, RESET);

    printf("\tEnter the level: ");
    fscanf(stdin, "%d", &stat);
    fprintf(file, "%d\n", stat);
    printf("\t%sSet level to %d%s\n", GREEN, stat, RESET);

    fputs("END_ITEM\n", file);

    FLUSH()

    id++;
  }

  printf("SoulWeapon items created and saved to ");

}

/**
 * Creates armor of num items, depending on type, writing them to the appropriate file.
 * @param file The armor piece file
 * @param num How many of the pieces to create
 * @param type The type of armor
 */
static void createArmor(FILE* file, int num, item_t type) {
  int id = 0;

  str line = NULL;
  size_t n;

  int stat;

  while (id != num) {
    printf("For %s %d:\n", itemToString(type), id);

    fprintf(file, "%d\n", id);
    printf("\t%sSet %s id to %d\n%s", GREEN, itemToString(type), id, RESET);

    fputs("1\n", file);
    printf("\t%sSet count to 1\n%s", GREEN, RESET);

    printf("\tEnter name: ");
    getline(&line, &n, stdin);
    fputs(line, file);
    printf("\t%sSet %s name to %s%s", GREEN, itemToString(type), line, RESET);

    printf("\tEnter the accuracy: ");
    fscanf(stdin, "%d", &stat);
    fprintf(file, "%d\n", stat);
    printf("\t%sSet accuracy stat to %d%s\n", GREEN, stat, RESET);

    printf("\tEnter the defense: ");
    fscanf(stdin, "%d", &stat);
    fprintf(file, "%d\n", stat);
    printf("\t%sSet defense stat to %d%s\n", GREEN, stat, RESET);

    printf("\tEnter the level: ");
    fscanf(stdin, "%d", &stat);
    fprintf(file, "%d\n", stat);
    printf("\t%sSet level to %d%s\n", GREEN, stat, RESET);

    fputs("END_ITEM\n", file);

    FLUSH()

    id++;
  }

  printf("%s items created and saved to ", itemToString(type));
}

/**
 * Creates hp kits of num items, writing them to the appropriate file.
 * @param file The hp_kit.item file
 * @param num How many hp kits to create
 */
static void createHPKit(FILE* file, int num) {
  int id = 0;

  str line = NULL;
  size_t n;

  str desc = NULL;
  int count;

  while (id != num) {
    printf("For HP Kit %d:\n", id);

    fprintf(file, "%d\n", id);
    printf("\t%sSet HP Kit id to %d\n%s", GREEN, id, RESET);

    printf("How many? ");
    fscanf(stdin, "%d", &count);
    fprintf(file, "%d\n", count);
    printf("\t%sSet HP Kit count to %d%s\n", GREEN, count, RESET);
    FLUSH()

    printf("What type? (Deka=0|Mega=1|Peta=2) ");
    getline(&line, &n, stdin);
    fputs(line, file);
    printf("\t%sSet HP Kit type to %s%s", GREEN, line, RESET);

    switch (*line) {
      case 0x30: // Deka
        desc = "This is a description for deka HP Kits. Heals for \n";
        break;
      case 0x31: // Mega
        desc = "Description for mega HP Kits. Heals for \n";
        break;
      case 0x32: // Peta
        desc = "This is some description for peta HP Kits. Heals for \n";
        break;
      default:
        desc = "UNRECOGNIZED TYPE\n";
        break;
    }
  
    fputs(desc, file);
    printf("\t%sSet description%s\n", GREEN, RESET);
  
    fputs("END_ITEM\n", file);

    id++;
  }

  printf("HP Kit items created and saved to ");
}

/**
 * Creates armor of num items, depending on type, writing them to the appropriate file.
 * @param file The upgrade file
 * @param num How many of upgrades to create
 * @param type The type of upgrade
 */
static void createUpgrade(FILE* file, int num, item_t type) {
  int id = 0;

  int count;
  char rank;

  while (id != num) {
    printf("For %s %d:\n", itemToString(type), id);

    fprintf(file, "%d\n", id);
    printf("\t%sSet %s id to %d\n%s", GREEN, itemToString(type), id, RESET);

    printf("How many? ");
    fscanf(stdin, "%d", &count);
    fprintf(file, "%d\n", count);
    printf("\t%sSet %s count to %d%s\n", GREEN, itemToString(type), count, RESET);
    FLUSH()

    printf("What rank? (B|A|S) ");
    fscanf(stdin, "%c", &rank);
    fprintf(file, "%c\n", rank);
    printf("\t%sSet rank to %c%s\n", GREEN, rank, RESET);
    FLUSH()

    str descRank = NULL;
    str descType = NULL;
    switch (type) {
      case WEAPON_UPGRADE_MATERIALS_T:
        descType = "SoulWeapons";
        break;
      case ARMOR_UPGRADE_MATERIALS_T:
        descType = "Armor";
        break;
      default:
        descType = "Unrecognized-Type";
        break;
    }
    switch (rank) {
      case 'B':
        descRank = "low-leveled";
        break;
      case 'A':
        descRank = "medium-leveled";
        break;
      case 'S':
        descRank = "high-leveled";
        break;
      default:
        descRank = "unrecognized-rank";
        break;
    }

    fprintf(file, "Used for upgrading %s %s\n", descRank, descType);
    printf("%sSet description%s\n", GREEN, RESET);

    fputs("END_ITEM\n", file);

    id++;
  }

  printf("%s items created and saved to ", itemToString(type));
}

/**
 * Creates slimes of num items, writing them to the appropriate file.
 * @param file The slime.item file
 * @param num How many slimes to create
 */
static void createSlime(FILE* file, int num) {
  int id = 0;

  int count;

  while (id != num) {
    printf("For Slime %d:\n", id);

    fprintf(file, "%d\n", id);
    printf("\t%sSet Slime id to %d\n%s", GREEN, id, RESET);

    printf("How many? ");
    fscanf(stdin, "%d", &count);
    fprintf(file, "%d\n", count);
    printf("\t%sSet Slime count to %d%s\n", GREEN, count, RESET);
  
    fputs("Slime. Sold for 100 DZ\n", file);
    printf("\t%sSet description%s\n", GREEN, RESET);
  
    fputs("END_ITEM\n", file);

    FLUSH()

    id++;
  }

  printf("Slime items created and saved to ");
}


int main(int argc, char const* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: CreateItem [type_of_item]\n");
    fprintf(stderr, "SoulWeapon: 0; Helmet: 1; Shoulder Guard: 2; Chestplate: 3; Boots: 4; ");
    fprintf(stderr, "HP Kits: 5; Weapon Upgrade Material: 6; Armor Upgrade Material: 7; Slime: 8\n");
    return 1;
  }

  item_t itemType = (item_t) atoi(argv[1]);
  int items;

  printf("How many of %s item? ", itemToString(itemType));
  fscanf(stdin, "%d", &items);
  FLUSH()

  FILE* itemFile = NULL;
  str filename = NULL;
  str itemName = NULL;

  switch (itemType) {
    case SOULWEAPON_T:
      itemName = "soulweapon";
      break;
    case HELMET_T:
      itemName = "helmet";
      break;
    case SHOULDER_GUARD_T:
      itemName = "shoulder_guard";
      break;
    case CHESTPLATE_T:
      itemName = "chestplate";
      break;
    case BOOTS_T:
      itemName = "boots";
      break;
    case HP_KITS_T:
      itemName = "hp_kit";
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
      itemName = "weapon_upgrade";
      break;
    case ARMOR_UPGRADE_MATERIALS_T:
      itemName = "armor_upgrade";
      break;
    case SLIME_T:
      itemName = "slime";
      break;
    default:
      itemName = "UNKNOWN";
      break;
  }
  
  filename = (str) malloc(18 + strlen(itemName));
  sprintf(filename, "./out/items/%s.item", itemName);
  itemFile = fopen(filename, "w");
  if (!itemFile) handleError(ERR_IO, FATAL, "Could not create %s item file!\n", itemName);

  switch (itemType) {
    case SOULWEAPON_T:
      createSoulWeapon(itemFile, items);
      break;
    case HELMET_T:
    case SHOULDER_GUARD_T:
    case CHESTPLATE_T:
    case BOOTS_T:
      createArmor(itemFile, items, itemType);
      break;
    case HP_KITS_T:
      createHPKit(itemFile, items);
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
    case ARMOR_UPGRADE_MATERIALS_T:
      createUpgrade(itemFile, items, itemType);
      break;
    case SLIME_T:
      createSlime(itemFile, items);
      break;
    default:
      fclose(itemFile);
      free(filename);
      return 1;
      break;
  }

  printf("%s!\n", filename);

  fclose(itemFile);
  free(filename);

  return 0;
}