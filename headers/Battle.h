#ifndef _BATTLE_H
#define _BATTLE_H

#ifdef _WIN64
#include "unistd.h" // For compiling in Windows MSVC
#include <windows.h> // Using sleep(). Note, this sleep takes in ms
#else
#include <unistd.h> // Note, the sleep takes in sec
#endif

#include "SoulWorker.h"


extern SoulWorker* player;
extern Maze* maze;
extern uchar from;

void battleEnemy(Enemy* enemy);

/**
 * Wrapper around sleep function for Windows and Linux.
 * @param ms Amount of milliseconds to sleep for
 */
void ssleep(int ms);

#endif