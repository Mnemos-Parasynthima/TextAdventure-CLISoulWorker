#include <stdio.h>
#include <stdlib.h>

#include "SoulWorker.h"
#include "Error.h"

#define NO_ITEM 0x0

SoulWorker* initSoulWorker(char* name) {
  SoulWorker* sw = (SoulWorker*) malloc(sizeof(SoulWorker));
  if (sw == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for the player!\n");  

  sw->name = name;
  sw->hp = 100;
  sw->xp = 0;
  sw->invCount = 0;
  sw->dzenai = 0;
  sw->maxHP = 100;
  

  for(int i = 0; i < INV_CAP; i++) {
    sw->inv[i]._item = NO_ITEM;
    sw->inv[i].count = NO_ITEM;
  }

  // Add gear slots, and weapon; maybe artifacts???

  return sw;
}


bool addToInv(SoulWorker* sw, item_t loot) {
  Item* item = NULL;

  // Look for item
  for (int i = 0; i < INV_CAP; i++) {
    if (sw->inv[i]._item == loot) {
      item = &(sw->inv[i]);

      break;
    }
  }

  if (item == NULL) { // Item not in inventory
    // Make sure enough space
    if (sw->invCount == INV_CAP) {
      printf("Your inventory is full! You must delete some items!\n");

      return false;
    }

    // Enough space, add it
    for (int i = 0; i < INV_CAP; i++) {
      if (sw->inv[i]._item != SLIME || sw->inv[i]._item != KEY) {
        sw->inv[i]._item = loot;
        sw->inv[i].count = 1;
        sw->invCount++;

        printf("Item has been added to the inventory!\n");
        return true;
      }
    }
  }

  for (int i = 0; i < INV_CAP; i++) {
    if (sw->inv[i]._item == loot) {
      sw->inv[i].count++;

      printf("Item count has been increased!");
      return true;
    }
  }

  return false;
}

bool removeFromInv(SoulWorker* sw, item_t loot) {
  Item* item;

  // Look for item
  for (int i = 0; i < INV_CAP; i++) {
    if (sw->inv[i]._item == loot) {
      item = &(sw->inv[i]);

      break;
    }
  }

  if (item != NULL) {
    printf("You do not have that item in your inventory!\n");

    return false;
  }

  for (int i = 0; i < INV_CAP; i++) {
    if (sw->inv[i]._item == loot) {
      if (sw->inv[i].count > 1) {
        sw->inv[i].count--;
        sw->invCount--;
      } else {
        sw->inv[i]._item = NO_ITEM;
        sw->inv[i].count = NO_ITEM;
      }

      return true;
    }
  }


  return false;
}

void viewInventory(SoulWorker* sw) {
  if (sw->invCount == 0) printf("You have no items.\n");
  else {
    printf("You have %d item%s: \n", sw->invCount, (sw->invCount == 1) ? "" : "s");

    for (int i = 0; i < INV_CAP; i++) {
      if (sw->inv[i]._item != NO_ITEM) printf("\t%c x %d\n", sw->inv[i]._item, sw->inv[i].count);
    }
  }
}

void deleteSoulWorker(SoulWorker *sw) {
  if (sw == NULL) return;

  free(sw->name);
  free(sw);
  sw = NULL;
}