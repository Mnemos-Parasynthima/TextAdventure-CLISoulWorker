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
 * 
 */
void printSplashScreen() {
  printf("╔═════════════════════════════════════******═══════════════════════════════════════════════╗\n");
  printf("|                                                                                          |\n");
  printf("|%s                               ██████╗██╗     ██╗                                         %s|\n", BLINK, RESET);
  printf("║%s                              ██╔════╝██║     ██║                                         %s║\n", BLINK, RESET);
  printf("║%s                              ██║     ██║     ██║                                         %s║\n", BLINK, RESET);
  printf("*%s                              ██║     ██║     ██║                                         %s*\n", BLINK, RESET);
  printf("*%s                              ╚██████╗███████╗██║                                         %s*\n", BLINK, RESET);
  printf("║                                                                                          ║\n");
  printf("║%s  ███████╗ ██████╗ ██╗    ██╗██╗     ██╗    ██╗ ██████╗ ██████╗ ██╗  ██╗███████╗██████╗   %s║\n", BLINK, RESET);
  printf("*%s  ██╔════╝██╔═══██╗██║    ██║██║     ██║    ██║██╔═══██╗██╔══██╗██║ ██╔╝██╔════╝██╔══██╗  %s*\n", BLINK, RESET);
  printf("*%s  ███████╗██║   ██║██║    ██║██║     ██║ █╗ ██║██║   ██║██████╔╝█████╔╝ █████╗  ██████╔╝  %s*\n", BLINK, RESET);
  printf("║%s  ╚════██║██║   ██║██║    ██║██║     ██║███╗██║██║   ██║██╔══██╗██╔═██╗ ██╔══╝  ██╔══██╗  %s║\n", BLINK, RESET);
  printf("║%s  ███████║╚██████╔╝╚███████╔╝███████╗╚███╔███╔╝╚██████╔╝██║  ██║██║  ██╗███████╗██║  ██║  %s║\n", BLINK, RESET);
  printf("|%s  ╚══════╝ ╚═════╝  ╚══╝╚══╝ ╚══════╝ ╚══╝╚══╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝  %s|\n", BLINK, RESET);
  printf("|                                                                                          |\n");
  printf("╚══════════════════════════════════════******══════════════════════════════════════════════╝\n");

  printf("                           The Journey of the SoulWorker....                                \n");
  // Next two lines make sure the player only presses the enter key once
  // Using only getchar() makes it press two (for some reason)
  getchar();
  // Don't know why this works but it works
  ungetc('\n', stdin);

  FLUSH()
  system("clear"); // I DO NOT like this
}

/**
 * 
 */
void printWelcome() {
  printf("✧~~~~~~~~~~~~~~~~~~✧\n");
  printf("Welcome to Cloudream\n");
  printf("✧~~~~~~~~~~~~~~~~~~✧\n");
  // Add delay in print characters
  printf("      . . . .\n");

  printf("Cloudream, divided.\n");
  ssleep(1000);
  printf("North. East. South. %sWest%s...\n", PURPLE, RESET);
  ssleep(1000);
  printf("Which %sRosca%s hath forsaken\n", YELLOW, RESET);
  ssleep(1000);
  printf("To which %sKent%s then envores\n", RED, RESET);
  ssleep(1000);
  printf("\n\n");
}

/**
 * 
 */
void tutorial() {
  printf("*~%sRosca%s shows you how to navigate the New World~*\n", YELLOW, RESET);
  ssleep(500);

  // Maybe extract to a text file?

  printf("In Game Menu:\n");
  printf("\tTo view your stats: ('e')\n");
  printf("\tTo view the map: ('p')\n");
  printf("\tTo move ('m') to a direction: ('n', 'e', 's', 'w')\n");
  printf("\tTo save the game: ('s')\n");
  printf("\tTo open the inventory menu: ('k')\n");
  printf("\tTo open the skill menu: ('k')\n");
  printf("\tTo unequip all the gear: ('g')\n");
  printf("\tTo save and quit: ('q')\n");
  printf("\tTo view the main help message: ('h')\n");
  ssleep(500);

  printf("In Inventory Menu:\n");
  printf("\tTo view the item: ('i')\n");
  printf("\tTo view your inventory: ('v')\n");
  printf("\tTo use an item (opens item menu): ('u')\n");
  printf("\tTo close the inventory: ('q')\n");
  printf("\tTo view the inventory help message: ('h')\n");
  ssleep(500);

  printf("In Item Menu:\n");
  printf("\tTo sell the item (all items): ('s')\n");
  printf("\tTo equip (only for gear): ('e')\n");
  printf("\tTo upgrade (only for gear): ('u')\n");
  printf("\tTo heal (only for HP kits): ('h')\n");
  ssleep(500);

  printf("In Skill Menu:\n");
  printf("\tTo view skill info: ('i')\n");
  printf("\tTo view all skills and equipped skills: ('k')\n");
  printf("\tTo unlock a skill: ('n')\n");
  printf("\tTo set a skill to equipped skills: ('s')\n");
  printf("\tTo upgrade a skill: ('u')\n");
  printf("\tTo close the menu: ('q')\n");
  printf("\tTo view the skill menu help message: ('h')\n");
  ssleep(500);


  // Add more info????
  printf("%sRosca%s wishes you the best...\n\n", YELLOW, RESET);
  ssleep(500);
}

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
    // if (feof(intro)) break;
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
  
  printf("You find yourself in %s...\n", currRoom->info);

  while (true) {
    if (!currRoom->hasBoss && currRoom->enemy.enemy != NULL) {
      battleEnemy(currRoom->enemy.enemy);
      // Update currRoom in case player respawned at entrance
      // Prevent from getting the loot, if one exists
      currRoom = player->room;
    }

    if (currRoom->hasBoss && currRoom->enemy.boss != NULL) {
      bossBattle(currRoom->enemy.boss);

      // Transport to next maze

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
  if (argc < 2) exit(1); // running without launcher, exit silently

  // if ran in cmd, check it was done by the launcher
  const char* arg = argv[1];
  if (strncmp(arg, "-l", 2) != 0) {
    exit(1);
  }

  // Dev shortcuts
  bool noSplash = false, noIntro = false;

  if (argc == 3) {
    const char* arg1 = argv[2];
    if (strncmp(arg1, "-nosplash", 9) == 0) {
      noSplash = true;
    }
  }
  if (!noSplash) printSplashScreen();

  if (argc == 4) {
    const char* arg2 = argv[3];
    if (strncmp(arg2, "-nointro", 8) == 0) {
      noIntro = true;
    }
  }
  printWelcome();

  bool saveExists = detectSave();

  if (saveExists) {
    printf("~~~~The %sAkashic Records%s indicate change~~~~\n", YELLOW, RESET);
    printf("Do you wish to follow the Records? (yes|no) ");

    // char buff[5];
    // fgets(buff, sizeof(buff), stdin);
    str buff = (str) malloc(5); // max is 3 for "yes", plus newline, plus null
    fgets(buff, 5, stdin);

    str in = buff; // better name
    for(;*buff != '\0';buff++) *buff = tolower(*buff);

    if (strncmp(in, "yes", 3) == 0) {
      free(in);

      printf("Reading Akashic Records...\n");
      loadGame();

      printf("%sRosca%s welcomes you back, %s...\n", YELLOW, RESET, player->name);

      loop();
    } else if (strncmp(in, "no", 2) == 0) {
      printf("The Akashic Records shall start anew...\n");
    } else {
      printf("No such response exists for the Records. Starting anew...\n\n");
    }

    free(in);
  }

  maze = initMaze("./data/maps/map.json");

  printf("What shall the Records name you, birthing %sSoul%s? ", CYAN, RESET);

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

  printf("%sRosca%s cordially welcomes you, %sSoulWorker %s%s, to Cloudream...\n\n", YELLOW, RESET, CYAN, player->name, RESET);
  ssleep(1000);

  tutorial();
  if (!noIntro) introStory();
  loop();


  return 0;
}