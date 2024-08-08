#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "Setup.h"

/**
 * Removes an item from the given room.
 * @param room The target room
 * @return True if it was removed, false otherwise
 */
bool removeItemFromMap(Room* room);

/**
 * Removes an enemy from the given room.
 * @param room The target room
 * @return True if it was removed, false otherwise
 */
bool removeEnemyFromMap(Room* room);