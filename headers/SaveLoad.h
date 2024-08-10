#include "Maze.h"
#include "SoulWorker.h"


extern SoulWorker* player;
extern Maze* maze;

/**
 * Saves the current game state.
 */
void saveGame();

void loadGame(const char* savefile);