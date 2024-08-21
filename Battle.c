#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#include "Battle.h"


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
  printf("hitroll: %3.2f\n", hitRoll);
  if (hitRoll > attacker->ACC) return 0;

  short baseDamage = 1 + (attacker->ATK - target->DEF);
  printf("baseDamage: %d\n", baseDamage);
  if (baseDamage < 1) baseDamage = 1;

  float critRoll = (float) rand() / RAND_MAX;
  printf("critroll: %3.2f\n", critRoll);
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
    printf("%s attacks for %d dmg!\n", player->name, playerAtk);

    if (playerAtk >= enemy->hp) { printf("%s defeated!\n", enemy->name); break; }

    enemy->hp -= playerAtk;

    ssleep(500);

    enemyAtk = getTotalDmg(enemy->stats, player->stats);
    printf("%s attacks for %d dmg!\n", enemy->name, enemyAtk);

    if (enemyAtk >= player->hp) { printf("Player defeated!\n"); defeat = true; break; }

    player->hp -= enemyAtk;

    ssleep(500);
  }

  if (defeat) {
    enemy->hp = enemyMaxHP;
    printf("Respawning to entrance...\n");
    player->room = maze->entry;
    player->hp = player->maxHP;
  } else {
    player->xp += enemy->xpPoints;
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