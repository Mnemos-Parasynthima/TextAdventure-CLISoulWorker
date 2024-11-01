#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "Keyboard.h"
#include "Error.h"
#include "SaveLoad.h"


#define CHAR_TO_INDEX(c) \
    ((c) == 'n' ? 0 : \
    (c) == 'e' ? 1 : \
    (c) == 's' ? 2 : \
    3)



typedef enum {
  MOVE_NORTH = 'n',
  MOVE_EAST = 'e',
  MOVE_SOUTH = 's',
  MOVE_WEST = 'w'
} Movement;

typedef enum {
  INV_SELL = 's', // Sell item
  INV_INFO = 'i', // View item info
  INV_QUIT = 'q', // Exit inventory
  INV_SHOW = 'v', // Show inventory (again)
  INV_EQUIP = 'e', // Equip gear
  INV_HELP = 'h' // Available options for inv menu
} Inventory;

typedef enum {
  SKILL_INFO = 'i', // View specific skill info
  SKILL_SHOW = 'k', // Show all skills (again)
  SKILL_UNLOCK = 'n', // Unlock a skill
  SKILL_SET = 's', // Set a skill to the skill slots
  SKILL_UPGRADE = 'u', // Upgrade a skill
  SKILL_QUIT = 'q', // Exit skill page
  SKILL_HELP = 'h' // Available options for skill menu
} Skills;

typedef enum {
  MOVEMENT_H,
  ACTIONS_H,
  INVENTORY_H,
  SKILLS_H
} HELP_T;


uchar from; // Find much better naming. Basically, where the player headed to upon proper direction


/**
 * Valides the given exit direction. That is, whether the direction points to a closed exit.
 * @param dir The direction exit to validate
 * @param room The current room
 * @return True if valid, false otherwise
 */
static bool validExit(Movement dir, Room* room) {
  if ((dir == MOVE_NORTH && room->exits[0] == (void*) ((long long)NO_EXIT)) ||
      (dir == MOVE_EAST && room->exits[1] == (void*) ((long long)NO_EXIT)) ||
      (dir == MOVE_SOUTH && room->exits[2] == (void*) ((long long)NO_EXIT)) ||
      (dir == MOVE_WEST && room->exits[3] == (void*) ((long long)NO_EXIT))) {

    return false;
  }

  return true;
}

static uint getPrice(byte lvl, item_t type) {
  /**
   * Pricing Model
   * Each gear piece has a certain weight relative to each other.
   * For a given piece, there is a possible range of prices, dependent on its level (taking into account the weight)
   * At gametime (not runtime), it will select a random price from the range
   * The range will increase as the level increases.
   * For example, at level 1, a soulweapon can cost between 23 and 28 while a helmet can cost between 19 and 21.
   * Then at level 2, a soulweapon can cost between 25 and 31 while a helmet can cost between 21 and 23.
   * Then at gametime, the lvl 1 SW can cost 25 and the helmet can cost 19
   *   while the lvl 2 SW can still cost 25 and the helmet can cost 23.
   * However, at levels 15, 30, 45, 60, 75, and 100, which are reserved for legendary gear
   *   the price will be constant at 1.
   * There's a chance the legenary status/rarity will be included alongside the gear piece,
   *   but for now, it is as is.
   */

  // TODO: Play around with numbers and formula

  double weight;
  uint minPrice, maxPrice, price;

  switch (type) {
    case SOULWEAPON_T: weight = 1.0; break;
    case HELMET_T: weight = 0.6; break;
    case SHOULDER_GUARD_T: weight = 0.4; break;
    case CHESTPLATE_T: weight = 0.7; break;
    case BOOTS_T: weight = 0.5; break;
    default: break;
  }

  uint baseMinPrice = 20 + lvl * 2;
  uint baseMaxPrice = 25 + lvl * 3;
  minPrice = (uint) (baseMinPrice * weight);
  maxPrice = (uint) (baseMaxPrice * weight);

  price = minPrice + rand() % (maxPrice - minPrice + 1);

  return price;
}

static void sellItem(Item* item, ushort count) {
  uint dz = 0;

  switch (item->type) {
    case SOULWEAPON_T:
      SoulWeapon* sw = (SoulWeapon*) item->_item;
      if (((sw->lvl % 15 == 0) && sw->lvl < 90) || sw->lvl == 100) dz = 1; // Legendary (boss dropped) only 1 dz
      else dz = getPrice(sw->lvl, SOULWEAPON_T);
      break;
    case HELMET_T:
    case SHOULDER_GUARD_T:
    case CHESTPLATE_T:
    case BOOTS_T:
      Armor* armor = (Armor*) item->_item;
      if (((armor->lvl % 15 == 0) && armor->lvl < 90) || armor->lvl == 100) dz = 1; // Legendary (boss dropped) only 1 dz
      else dz = getPrice(armor->lvl, item->type);
      break;
    case HP_KITS_T:
      HPKit* hpKit = (HPKit*) item->_item;
      if (hpKit->type == DEKA) dz = 45;
      else if (hpKit->type == MEGA) dz = 55;
      else dz = 75;
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
    case ARMOR_UPGRADE_MATERIALS_T:
      Upgrade* upgrade = (Upgrade*) item->_item;
      if (upgrade->rank == B) dz = 205;
      else if (upgrade->rank == A) dz = 315;
      else dz = 500;
      break;
    case SLIME_T:
      dz = 100;
    default:
      break;
  }

  uint total = dz * count;
  player->dzenai += total;

  str itemName = getItemName(item);
  printf("%d %s has been sold for %d dzenai!\n", count, itemName, total);

  if (item->type >= HP_KITS_T && item->type <= ARMOR_UPGRADE_MATERIALS_T) free(itemName);

  removeFromInv(player, item, count);
}

static Item* validItem(byte itemI) {
  if (itemI < 1 || itemI > INV_CAP) return NULL;
  if (player->inv[itemI-1]._item == NULL) return NULL;

  return &(player->inv[itemI-1]);
}

/**
 * Validates the given direction. That is, whether the direction is a direction at all.
 * @param dir The direction to validate
 * @param room The current room
 * @return True if valid, false otherwise
 */
static bool validMove(Movement* dir, Room* room) {
  if (*dir == MOVE_NORTH || *dir == MOVE_EAST || *dir == MOVE_SOUTH || *dir == MOVE_WEST) {
    while (!validExit(*dir, room)) {
      printf("That direction is closed. Try another direction! ");

      *dir = getchar();
      FLUSH()
      *dir = tolower(*dir);
    }

    // Temporary fix for bug where player inputs not a direction when re-prompted after closed exit
    //  and it passes all current checks, leading to access to an invalid-nonexisting direction, leading to segfault
    // This does not look clean or proper code at the moment.
    // I give up (for now?) in trying to figure this out
    if (*dir == MOVE_NORTH || *dir == MOVE_EAST || *dir == MOVE_SOUTH || *dir == MOVE_WEST) return true;

    return false;
  }
  
  return false;
}

static bool validSkillAction(Skills opt) {
  return (opt == SKILL_INFO || opt == SKILL_SHOW || opt == SKILL_UNLOCK || 
      opt == SKILL_SET || opt == SKILL_UPGRADE || opt == SKILL_QUIT || opt == SKILL_HELP);
}

static bool validInvAction(Inventory inv) {
  return (inv == INV_SELL || inv == INV_INFO || inv == INV_HELP || inv == INV_QUIT || inv == INV_SHOW || inv == INV_EQUIP);
}

static int validSkillNum(char buffer[3]) {
  int num = (uint) atoi(buffer);

  if (num > TOTAL_SKILLS || num < 0) {
    printf("Not a skill number!\n");
    return -1;
  }

  return num;
}

static Item* getItemFromPos() {
  uchar _item = getchar();
  FLUSH()
  uchar itemI = _item - '0';

  Item* item = validItem(itemI);
  while (item == NULL) {
    printf("That is not a valid inventory entry!\n Try again! ");
    _item = getchar();
    FLUSH()
    uchar itemI = _item - '0';

    item = validItem(itemI);
  }

  return item;
}

static void displayHelp(HELP_T type) {
  if (type == MOVEMENT_H) {
    printf("Directions are north ('n'), east ('e'), south ('s'), and west ('w').");
  } else if (type == INVENTORY_H) {
    printf("Possible actions are:\n");
    printf("\t Sell item ('s')\n");
    printf("\t View item info ('i')\n");
    printf("\t Close inventory ('q')\n");
    printf("\t View inventory ('v')\n");
    printf("\t Equip gear ('e')\n");
    printf("\t Help message ('h')\n");
  } else if (type == SKILLS_H) {
    printf("Possible actions are:\n");
    printf("\t View skill info ('i')\n");
    printf("\t View skills ('k')\n");
    printf("\t Unlock skill ('n')\n");
    printf("\t Set skill ('s')\n");
    printf("\t Upgrade skill ('u')\n");
    printf("\t Close skill menu ('q')\n");
    printf("\t Help message ('h')\n");
  } else { // type == ACTIONS
    printf("Possible actions are:\n");
    printf("\t Open Inventory ('i')\n");
    printf("\t Open Skill Menu ('k')\n");
    printf("\t Move ('m')\n");
    printf("\t Save ('s')\n");
    printf("\t Save and Quit ('q')\n");
    printf("\t View self ('e')\n");
    printf("\t Unequip all gear ('g')\n");
    printf("\t Help message ('h')\n");
  }
}

/**
 * Saves te game and quits.
 */
static void quitGame() {
  saveGame();
  deleteSoulWorker(player);
  deleteMaze(maze);
  exit(0);
}

bool performAction(Commands action) {
  if (action == WALK) {
    printf("What direction do you want to move? ");
    Movement dir = getchar();
    FLUSH()
    dir = tolower(dir);

    while(!validMove(&dir, player->room)) {
      printf("That is not a direction. Try again! For a list of acceptable direction, type 'h'. ");

      dir = getchar();
      FLUSH()
      dir = tolower(dir);

      if (dir == 'h') displayHelp(MOVEMENT_H);
    }

    from = (uchar) dir;

    printf("Entering room...\n");

    player->room = player->room->exits[CHAR_TO_INDEX(dir)];

    printf("You are in %s...\n", player->room->info);

    if (player->room == (void*)(long long)(NO_EXIT)) handleError(ERR_MEM, FATAL, "Cannot access exit!\n");

    // printf("You are in %s...\n", currRoom->info);
  } else if (action == OPEN_INVENTORY) {
    viewInventory(player);

    printf("Inventory: What do you want to do? ");
    Inventory inv = getchar();
    FLUSH()
    inv = tolower(inv);

    while (inv != INV_QUIT) {
      while(!validInvAction(inv)) {
        printf("That is not a valid action. Try again! For a list of acceptable actions, type 'h'. ");

        inv = getchar();
        FLUSH()
        inv = tolower(inv);
      }

      if (inv == INV_SELL) {
        printf("What item? Use a number for its position. ");
        Item* item = getItemFromPos();

        printf("How many do you want to sell? ");
        uchar _count = getchar();
        FLUSH()
        uchar count = _count - '0';

        while (count < 0 || count > item->count) {
          printf("Invalid count! Try again! ");
          _count = getchar();
          FLUSH()
          count = _count - '0';
        }        

        sellItem(item, count);
      } else if (inv == INV_INFO) {
        printf("What item do you want to inspect? Use a number for its position. ");
        Item* item = getItemFromPos();

        printf("Viewing item info...\n");

        switch (item->type) {
          case SOULWEAPON_T:
            SoulWeapon* sw_t = (SoulWeapon*) item->_item;
            displaySoulWeapon(sw_t);
            break;
          case HELMET_T:
          case SHOULDER_GUARD_T:
          case CHESTPLATE_T:
          case BOOTS_T:
            Armor* armor_t = (Armor*) item->_item;
            displayArmor(armor_t);
            break;
          case HP_KITS_T:
            HPKit* hp_t = (HPKit*) item->_item;
            displayHPKit(hp_t);
            break;
          case WEAPON_UPGRADE_MATERIALS_T:
          case ARMOR_UPGRADE_MATERIALS_T:
            Upgrade* up_t = (Upgrade*) item->_item;
            displayUpgrade(up_t);
            break;
          case SLIME_T:
            Slime* slime_t = (Slime*) item->_item;
            displaySlime(slime_t);
            break;
          default:
            printf("NOT AN ITEM\n");
            break;
        }
      } else if (inv == INV_EQUIP) {
        if (player->invCount == 0) printf("There are no items to equip!\n");
        else {
          printf("What do you want to equip? Use a numer for its position. ");
          Item* item = getItemFromPos();
          
          if (item->type < SOULWEAPON_T || item->type > BOOTS_T) {
            printf("That is not an equippable gear!\n");
          } else equipGear(player, item);
        }
      } else if (inv == INV_HELP) displayHelp(INVENTORY_H);
      else if (inv == INV_SHOW) viewInventory(player);
      else if (inv == INV_QUIT) {
        printf("Closing inventory\n");
        break;
      }

      printf("Inventory: What do you want to do? ");
      inv = getchar();
      FLUSH()
      inv = tolower(inv);
    } 
  } else if (action == OPEN_SKILLS) {
    viewSkills(player->skills);

    printf("Skill Menu: What do you want to do? ");
    Skills action = getchar();
    FLUSH()
    action = tolower(action);

    while (action != SKILL_QUIT) {
      while (!validSkillAction(action)) {
        printf("That is not a valid action. Try again! For a list of acceptable actions, type 'h'. ");

        action = getchar();
        FLUSH()
      }

      if (action == SKILL_SET) {
        // Get the skill number to set
        printf("What skill do you want to set? Use the skill number. ");
        char buffer[3]; // num can be up to 2 digits + /0
        if( !fgets(buffer, 3, stdin)) { printf("Could not get input!\n"); break; }
        // FLUSH()

        int skillNum = validSkillNum(buffer);
        if (skillNum == -1) break;
          // Check case that the skill is unlocked
        if (!isSkillUnlocked(player->skills, skillNum)) { printf("Skill has not been unlocked!\n"); break; }

        // Get the slot to set at
        printf("Which slot? [1-5] ");
        char slot = getchar();
        FLUSH()

        int slotNum = slot - 0x30;
        if (slotNum <= 0 || slotNum > 5) { printf("Invalid slot!\n"); break; }

        setSkill(player->skills, &player->skills->skills[skillNum], (uint) slotNum);
      } else if (action == SKILL_INFO) {
        printf("Viewing skill!\n");

        // Get the skill number to view
        printf("What skill do you want to see? Use the skill number. ");
        char buffer[3]; // num can be up to 2 digits + /0
        if( !fgets(buffer, 3, stdin)) { printf("Could not get input!\n"); break; }
        // FLUSH()

        int skillNum = validSkillNum(buffer);
        if (skillNum == -1) break;

        viewSkill(&player->skills->skills[skillNum - 1]);
      } else if (action == SKILL_UNLOCK) {
        // Get the skill number to unlocked
        printf("What skill do you want to unlock? Use the skill number. ");
        char buffer[3]; // num can be up to 2 digits + /0
        if (!fgets(buffer, 3, stdin)) { printf("Could not get input!\n"); break; }

        int skillNum = validSkillNum(buffer);
        if (skillNum == -1) break;
        // Check if it's already unlocked
        if (isSkillUnlocked(player->skills, skillNum)) { printf("Skill has already been unlocked!\n"); break; }

        skillUnlock(player->skills, skillNum);        
      } else if (action == SKILL_UPGRADE) {
        printf("Upgrading skill!\n");
      } else if (action == SKILL_HELP) displayHelp(SKILLS_H);
      else if (action == SKILL_SHOW) viewSkills(player->skills);
      else if (action == SKILL_QUIT) {
        printf("Closing skill menu\n");
        break;
      }


      printf("Skills: What do you want to do? ");
      action = getchar();
      FLUSH()
      action = tolower(action);
    }
  } else if (action == HELP) displayHelp(ACTIONS_H);
    else if (action == SAVE) saveGame();
    else if (action == QUIT) quitGame();
    else if (action == INFO) viewSelf(player);
    else if (action == UNEQUIP) unequipGear(player);
    else if (action == MAP) showMap(maze, player->room);
    else return false;

  return true;
}