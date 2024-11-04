#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#include "Battle.h"
#include "Error.h"


typedef enum {
  NORTH = 'n',
  EAST = 'e',
  SOUTH = 's',
  WEST = 'w'
} DIRECTION;


void ssleep(int ms) {
  #ifdef _WIN64
    Sleep(ms);
  #else
    usleep(ms * 1000);
  #endif
}


/**
 * When defeated, find the direction that the player came from and set that as the new room.
 */
static void returnRoom() {
  int idx;

  // Player chose to go `from`, so when retreating, go the opposite way
  // Ie the player chose to go south (2), decided to retreat, so they'll be going north (0)
  switch (from) {
    case NORTH:
      idx = 2;
      break;
    case EAST:
      idx = 3;
      break;
    case SOUTH:
      idx = 0;
      break;
    case WEST:
      idx = 1;
      break;
    default:
      idx = -1;
      break;
  }

  player->room = player->room->exits[idx];
}

/**
 * Calculates how much HP lost based on attacker and target stats
 * @param attacker The stats of the attacker
 * @param target The stats of the target
 * @return How much damage taken
 */
static ushort getTotalDmg(Stats* attacker, Stats* target) {
  float hitRoll = (float) rand() / RAND_MAX * (player->lvl * 3);
  // printf("hitroll: %3.2f\n", hitRoll);
  if (hitRoll > attacker->ACC) return 0;

  short baseDamage = 1 + (attacker->ATK - target->DEF);
  // printf("baseDamage: %d\n", baseDamage);
  if (baseDamage < 1) baseDamage = 1;

  float critRoll = (float) rand() / RAND_MAX;
  // printf("critroll: %3.2f\n", critRoll);
  if (critRoll <= attacker->ATK_CRIT) {
    baseDamage += ((ushort) baseDamage * attacker->ATK_CRIT_DMG) / 100;
  }

  return (ushort) baseDamage;
}

/**
 * Runs the auto-battle for the enemy
 * @param enemy The enemy to fight
 */
static void fight(Enemy* enemy) {
  ushort playerAtk, enemyAtk;

  bool defeat = false; // Player defeat
  uint enemyMaxHP = enemy->hp;

  while (true) {
    playerAtk = getTotalDmg(player->stats, enemy->stats);
    // printf("%s attacks for %d dmg!\n", player->name, playerAtk);

    if (playerAtk >= enemy->hp) { printf("%s defeated!\n", enemy->name); break; }

    enemy->hp -= playerAtk;

    // printf("%s: %d/%d\n", enemy->name, enemy->hp, enemyMaxHP);

    ssleep(500);

    enemyAtk = getTotalDmg(enemy->stats, player->stats);
    // printf("%s attacks for %d dmg!\n", enemy->name, enemyAtk);

    if (enemyAtk >= player->hp) { printf("Player defeated!\n"); defeat = true; break; }

    player->hp -= enemyAtk;

    // printf("%s: %d/%d\n", player->name, player->hp, player->maxHP);

    ssleep(500);
  }

  if (defeat) {
    enemy->hp = enemyMaxHP;
    printf("Respawning to entrance...\n");
    player->room = maze->entry;
    player->hp = player->maxHP;
  } else {
    updateXP(player, enemy->xpPoints);
    player->skills->totalSkillPoints += 1;
    deleteEnemyFromMap(player->room, false);
  }
}

void battleEnemy(Enemy* enemy) {
  printf("You encountered %s!\n", enemy->name);
  displayEnemyStats(enemy);
  printf("Do you want to fight or retreat? [f|r] ");

  char decision = getchar();
  decision = tolower(decision);
  FLUSH()

  while (!(decision == 'f' || decision == 'r')) {
    printf("Invalid choice! What do you want to do?.... ");

    decision = getchar();
    decision = tolower(decision);
    FLUSH()
  }

  if (decision == 'f') {
    printf("FIGHT!\n");
    fight(enemy);
  } else {
    printf("Retreating!\n");

    returnRoom();

    printf("You are back in %s...\n", player->room->info);

    return;
  }  
}

static void displayAttackOptions() {
  /**
   * What are you going to do?
   * [0] Basic attack
   * [1] SKILL 1
   * [3] SKILL 2
   * [4] SKILL 5
   * [5] SKILL 7
   * [6] SKILL 9
   * : 
   */

  printf("[0] Basic attack\n");

  Skill** equipped = player->skills->equippedSkills;

  for (int i = 0; i < EQUIPPED_SKILL_COUNT; i++) {
    if (equipped[i] != NULL) {
      printf("[%d] %s\n", i + 1, equipped[i]->name);
    }
  }
}

static bool validAttack(char attack) {
  if (attack == '0') return true;

  for (int i = 0; i < EQUIPPED_SKILL_COUNT; i++) {
    if ((player->skills->equippedSkills[i] != NULL) && ((attack - 0x30) == i + 1)) return true;
  }

  return false;
}

void bossBattle(Boss* boss) {
  ushort playerAtk, enemyAtk;

  bool defeat = false; // Player defeat
  uint bossMaxHP = boss->base.hp;

  printf("BOSS ENCOUNTERED!!\n");
  displayEnemyStats(&(boss->base));
  printf("Retreat is not an option!\n");

  while (true) {
    printf("What are you going to do?\n");
    displayAttackOptions();
    printf(": ");
    uchar attack = getchar();
    FLUSH()
    while (!validAttack(attack)) {
      printf("That is not an attack!\n");
      printf(": ");
      attack = getchar();
      FLUSH()
    }

    playerAtk = getTotalDmg(player->stats, boss->base.stats);
    printf("You dealt %d DMG!\n", playerAtk);

    if (playerAtk >= boss->base.hp) { printf("%s defeated!\n", boss->base.name); break; }

    boss->base.hp -= playerAtk;
    printf("%s: %d/%d\n", boss->base.name, boss->base.hp, bossMaxHP);

    ssleep(500);

    enemyAtk = getTotalDmg(boss->base.stats, player->stats);
    printf("You received %d DMG!\n", enemyAtk);

    if (enemyAtk >= player->hp) { printf("Player defeated!\n"); defeat = true; break; }

    player->hp -= enemyAtk;
    printf("%s: %d/%d\n", player->name, player->hp, player->maxHP);
  }

  if (defeat) {
    boss->base.hp = bossMaxHP;
    printf("Respawning to entrance...\n");
    player->room = maze->entry;
    player->hp = player->maxHP;
  } else {
    updateXP(player, boss->base.xpPoints);
    player->skills->totalSkillPoints += 3;

    // Need to create Item* in order to use addToInv
    Item* gearItem = (Item*) malloc(sizeof(Item));
    if (gearItem == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for gear item!\n");

    gearItem->count = 1;
    
    // TODO: Add way to save unpicked gear from boss
    /** 
     * In the event that the gear cannot be added to the inv (mainly because no more space in inv)
     * Need to somehow save it
     * Future me issue
     * Maybe by storing pointers somewhere (maybe in the room), and when user passes by the room again
     * it prompts if to pick up dropped loot.
     * For now, if no more inv space, just remove the dropped items
     */

    gearItem->type = SOULWEAPON_T;
    gearItem->_item = (void*) boss->gearDrop.sw;
    if (addToInv(player, gearItem)) printf("SoulWeapon added!\n");
    else { printf("Could not add SoulWeapon!\n"); goto end; }

    gearItem->type = HELMET_T;
    gearItem->_item = (void*) boss->gearDrop.helmet;
    if (addToInv(player, gearItem)) printf("Helmet added!\n");
    else { printf("Could not add helmet!\n"); goto end; }

    gearItem->type = SHOULDER_GUARD_T;
    gearItem->_item = (void*) boss->gearDrop.guard;
    if (addToInv(player, gearItem)) printf("Shoulder guard added!\n");
    else { printf("Could not add shoulder guard!\n"); goto end; }

    gearItem->type = CHESTPLATE_T;
    gearItem->_item = (void*) boss->gearDrop.chestplate;
    if (addToInv(player, gearItem)) printf("Chestplate added!\n");
    else { printf("Could not add chestplate!\n"); goto end; }

    gearItem->type = BOOTS_T;
    gearItem->_item = (void*) boss->gearDrop.boots;
    if (addToInv(player, gearItem)) printf("Boots added!\n");
    else { printf("Could not add boots!\n"); goto end; }

    end:
    // No longer need gearItem, free it
    // Do not free gearItem->_item since it points to boss->gearDrop.boots
    // which the player inv owns the pointer (thus the struct) now
    // NOTE: see prior big comment
    free(gearItem);
    gearItem = NULL;

    deleteEnemyFromMap(player->room, false);
  }
}