#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#include "Error.h"
#include "Keyboard.h"
#include "SaveLoad.h"
#include "Battle.h"


SoulWorker* player;
Maze* maze;


/**
 * Detects whether there is a saved game so the player can resume.
 * @return True if a saved game exists, false otherwise
 */
static bool detectSave() {
  FILE* mapSave = fopen("./data/saves/maps/map_save.json", "r");
  FILE* playerSave = fopen("./data/saves/player_save.json", "r");

  if (mapSave != NULL && playerSave != NULL) {
    fclose(mapSave);
    fclose(playerSave);

    return true;
  }

  return false;
}

static void introStory() {
  FILE* intro = fopen("./data/story/intro.dat", "r");

  if (intro == NULL) handleError(ERR_IO, FATAL, "Could not open story!\n");

  str line = NULL;
  size_t n = 0;

  printf("\n");
  while (!feof(intro)) {
    getline(&line, &n, intro);
    if (feof(intro)) break;
    ssleep(1000);
    printf("%s", line);
    line = NULL;
  }
  printf("\n\n");


  fclose(intro);
}


void loop() {
  Room* currRoom = player->room;
  Commands choice;
  
  printf("You are in %s...\n", currRoom->info);

  while (true) {
    if (!currRoom->hasBoss && currRoom->enemy.enemy != NULL) {
      battleEnemy(currRoom->enemy.enemy);
      // Update currRoom in case player respawned at entrance
      // Prevent from getting the loot, if one exists
      currRoom = player->room;
    }

    if (currRoom->hasBoss && currRoom->enemy.boss != NULL) {
      printf("Boss %s encountered!\n", currRoom->enemy.boss->base.name);

      displayEnemyStats(&(currRoom->enemy.boss->base));

      Gear* gearDrop = &(currRoom->enemy.boss->gearDrop);

      printf("It died from surprise! You are so powerful, wow!!\n");
      player->gear.boots = gearDrop->boots;
      player->gear.helmet = gearDrop->helmet;
      player->gear.guard = gearDrop->guard;
      player->gear.chestplate = gearDrop->chestplate;
      player->gear.sw = gearDrop->sw;

      // Temp
      player->stats->ATK += gearDrop->sw->atk;
      player->stats->ACC += (gearDrop->sw->acc + gearDrop->helmet->acc + gearDrop->guard->acc + gearDrop->chestplate->acc + gearDrop->boots->acc);
      player->stats->ATK_CRIT += gearDrop->sw->atk_crit;
      player->stats->ATK_CRIT_DMG += gearDrop->sw->atk_crit_dmg;
      player->stats->DEF += (gearDrop->helmet->def + gearDrop->guard->def + gearDrop->chestplate->def + gearDrop->boots->def);

      deleteEnemyFromMap(currRoom, false);
    }

    if (currRoom->loot != NULL) {
      str name = getItemName(currRoom->loot);
      printf("You found %d * %s!\n", currRoom->loot->count, name);
      bool added = addToInv(player, currRoom->loot);

      // Since some strings have been alloc'd, free them
      switch (currRoom->loot->type) {
        case HP_KITS_T:
        case WEAPON_UPGRADE_MATERIALS_T:
        case ARMOR_UPGRADE_MATERIALS_T:
          free(name);
          name = NULL;
        default:
          break;
      }

      if (added) removeItemFromMap(currRoom);
    }

    printf("What are you going to do?... ");
    choice = getchar();
    choice = tolower(choice);
    FLUSH()

    while(!performAction(choice)) {
      printf("That is not an action. Try again! For a list of acceptable actions, type 'h'!\n");
      choice = getchar();
      choice = tolower(choice);
      FLUSH()
    };

    currRoom = player->room;
  }
}


int main(int argc, char const *argv[]) {
  printf("Welcome to the Cloudream Adventure!\n");

  bool saveExists = detectSave();

  if (saveExists) {
    printf("A save has been detected! Do you want to load that save? (yes|no) ");

    // char buff[5];
    // fgets(buff, sizeof(buff), stdin);
    str buff = (char*) malloc(5); // max is 3 for "yes", plus newline, plus null
    fgets(buff, 5, stdin);

    str in = buff; // better name
    for(;*buff != '\0';buff++) *buff = tolower(*buff);

    if (strncmp(in, "yes", 3) == 0) {
      free(in);

      printf("Loading save...\n");
      loadGame();

      printf("Welcome back to Cloudream, %s!\n", player->name);

      loop();
    } else if (strncmp(in, "no", 2) == 0) {
      printf("Starting a new game...\n");
    } else {
      printf("Invalid decision. Starting a new game.\n");
    }

    free(in);
  }

  maze = initMaze("./data/maps/map.json");

  printf("Enter your name: ");

  str name = NULL;
  size_t n = 0;
  ssize_t nameLen = getline(&name, &n, stdin);
  *(name + nameLen - 1) = '\0';

  player = initSoulWorker(name);
  player->room = maze->entry;

  // printf("");
  // printf("Is\n\t%s\t\tyour name?...\n", name);
  // Do a yes or no, repeat type of thing
  // printf("")

  printf("Hello, %s! Welcome to Cloudream!\n", (player->name));
  // introStory();
  loop();


  return 0;
}