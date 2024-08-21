#include <stdio.h>
#include <ctype.h>



#include "Battle.h"

typedef enum {
  NORTH = 'n',
  EAST = 'e',
  SOUTH = 's',
  WEST = 'w'
} DIRECTION;


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

static ushort getTotalDmg(ushort attackerDmg, ushort targetDef) {
  ushort baseDamage = attackerDmg - (targetDef / 2);

  if (baseDamage < 1) baseDamage = 1;

  return baseDamage;
}

static void fight(Enemy* enemy) {
  while (true) {
    ushort playerAtk = getTotalDmg(player->stats->ATK, enemy->stats->DEF);
    printf("%s attacks for %d dmg!\n", player->name, playerAtk);
    enemy->hp -= playerAtk;

    ssleep(100);

    ushort enemyAtk = getTotalDmg(enemy->stats->ATK, player->stats->DEF);
    printf("%s attacks for %d dmg!\n", enemy->name, enemyAtk);
    player->hp -= enemyAtk;

    ssleep(100);
  }
}

void ssleep(int ms) {
  #ifdef _WIN64
    Sleep(ms);
  #else
    usleep(ms * 1000);
  #endif
}

void battleEnemy(Enemy* enemy) {
  // deleteEnemyFromMap(currRoom, false);  

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