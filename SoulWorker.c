#include "SoulWorker.h"

SoulWorker* initSoulWorker(char* name) {
  SoulWorker* sw = (SoulWorker*) malloc(sizeof(SoulWorker));

  sw->name = name;
  sw->hp = 100;
  sw->xp = 0;
  sw->invCount = 0;

  return sw;
}

bool addToInv(SoulWorker *sw, char loot) {
  if (sw->invCount == INV_CAP) {
    printf("Your inventory is full! You must delete some items!\n");

    return false;
  }

  for (int i = 0; i < INV_CAP; i++) {
    if (sw->inv[i] != 0) {
      sw->inv[i] = loot;
      sw->invCount++;

      printf("Item has been added to the inventory!\n");

      return true;
    }
  }

  return false;
}

bool removeFromInv(SoulWorker* sw, char loot) {
  for (int i = 0; i < INV_CAP; i++) {
    if (sw->inv[i] == loot) {
      sw->inv[i] = 0;
      sw->invCount--;

      return true;
    }
  }

  printf("You do not have that item in your inventory!\n");

  return false;
}

void viewInventory(SoulWorker *sw) {
  printf("You have %d item(s): \n", sw->invCount);

  for (int i = 0; i < sw->invCount; i++) {
    printf("\t%c\n", sw->inv[i]);
  }
}
