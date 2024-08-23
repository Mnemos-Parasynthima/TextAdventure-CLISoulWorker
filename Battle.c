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

void bossBattle(Boss* boss) {
  ushort playerAtk, enemyAtk;

  bool defeat = false; // Player defeat
  uint bossMaxHP = boss->base.hp;

  // printf("Boss %s encountered!\n", currRoom->enemy.boss->base.name);

  // displayEnemyStats(&(currRoom->enemy.boss->base));

  // Gear* gearDrop = &(currRoom->enemy.boss->gearDrop);

  // printf("It died from surprise! You are so powerful, wow!!\n");
  // player->gear.boots = gearDrop->boots;
  // player->gear.helmet = gearDrop->helmet;
  // player->gear.guard = gearDrop->guard;
  // player->gear.chestplate = gearDrop->chestplate;
  // player->gear.sw = gearDrop->sw;

  // deleteEnemyFromMap(currRoom, false);


  printf("BOSS ENCOUNTERED!!\n");
  displayEnemyStats(&(boss->base));
  printf("Retreat is not an option!\n");

  while (true) {
    printf("What are you going to do?\n[1] Basic attack\n: ");
    uchar attack = getchar();
    FLUSH()
    while (attack != '1') {
      printf("That is not an attack!\n");
      attack = getchar();
      FLUSH()
    }

    playerAtk = getTotalDmg(player->stats, boss->base.stats);
    printf("-%d!\n", playerAtk);

    if (playerAtk >= boss->base.hp) { printf("%s defeated!\n", boss->base.name); break; }

    boss->base.hp -= playerAtk;
    printf("%d/%d\n", boss->base.hp, bossMaxHP);

    ssleep(500);

    enemyAtk = getTotalDmg(boss->base.stats, player->stats);
    printf("-%d!\n", enemyAtk);

    if (enemyAtk >= player->hp) { printf("Player defeated!\n"); defeat = true; break; }

    player->hp -= enemyAtk;
    printf("%d/%d\n", player->hp, player->maxHP);
  }

  if (defeat) {
    boss->base.hp = bossMaxHP;
    printf("Respawning to entrance...\n");
    player->room = maze->entry;
    player->hp = player->maxHP;
  } else {
    updateXP(player, boss->base.xpPoints);

    Item* gearItem = (Item*) malloc(sizeof(Item));
    if (gearItem == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for gear item!\n");

    gearItem->count = 1;

    gearItem->type = SOULWEAPON_T;
    gearItem->_item = (SoulWeapon*) boss->gearDrop.sw;
    if(addToInv(player, gearItem)) printf("SoulWeapon added!\n");
    else { printf("Could not add SoulWeapon!\n"); deleteEnemyFromMap(player->room, false); return;}

    gearItem->type = HELMET_T;
    gearItem->_item = (Armor*) boss->gearDrop.helmet;
    addToInv(player, gearItem);

    gearItem->type = SHOULDER_GUARD_T;
    gearItem->_item = (Armor*) boss->gearDrop.guard;
    addToInv(player, gearItem);

    gearItem->type = CHESTPLATE_T;
    gearItem->_item = (Armor*) boss->gearDrop.chestplate;
    addToInv(player, gearItem);

    gearItem->type = BOOTS_T;
    gearItem->_item = (Armor*) boss->gearDrop.boots;
    addToInv(player, gearItem);

    deleteEnemyFromMap(player->room, false);
  }
}