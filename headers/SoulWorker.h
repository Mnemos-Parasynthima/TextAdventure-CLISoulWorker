#include "Setup.h"

typedef struct SoulWorker {
  char* name;
  int xp;
  int hp;
  char inv[10];
  int invCount;
  Room* room; // The current room that the player is in
} SoulWorker;

SoulWorker* initSoulWorker(char* name);