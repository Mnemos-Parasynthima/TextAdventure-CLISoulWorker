#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "Setup.h"

#define INV_CAP 50

typedef struct SoulWorker {
  char* name;
  int xp;
  int hp;
  char inv[INV_CAP];
  int invCount;
  Room* room; // The current room that the player is in
} SoulWorker;

SoulWorker* initSoulWorker(char* name);

bool addToInv(SoulWorker* sw, char loot);

bool removeFromInv(SoulWorker* sw, char loot);

void viewInventory(SoulWorker* sw);