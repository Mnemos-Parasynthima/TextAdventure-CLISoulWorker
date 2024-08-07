typedef struct SoulWorker {
  char* name;
  int xp;
  int hp;
  char inv[10];
  int invCount;
} SoulWorker;

SoulWorker* initSoulWorker(char* name);