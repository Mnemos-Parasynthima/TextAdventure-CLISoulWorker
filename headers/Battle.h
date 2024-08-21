#ifndef _BATTLE_H
#define _BATTLE_H

#include "SoulWorker.h"



extern SoulWorker* player;
// extern Maze* maze;
extern uchar from;

void battleEnemy(Enemy* enemy);

/**
 * Wrapper around sleep function for Windows and Linux.
 * @param ms Amount of milliseconds to sleep for
 */
// void ssleep(int ms);

#endif