#include <stdlib.h>

#include "SoulWorker.h"

SoulWorker* initSoulWorker(char* name) {
  SoulWorker* sw = (SoulWorker*) malloc(sizeof(SoulWorker));

  sw->name = name;
  sw->hp = 100;
  sw->xp = 0;

  return sw;
}