#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <stdbool.h>

#include "SoulWorker.h"


typedef enum {
  WALK = 'm',
  OPEN_INVENTORY = 'i',
  HELP = 'h',
  SAVE = 's',
  QUIT = 'q',
  INFO = 'e',
  GEAR = 'g',
  HEAL = 'a'
} Commands;

/**
 * Checks whether the given action is a valid action.
 * @param action The action to check
 * @return True if a valid action, false otherwise
 */
bool performAction(Commands action, SoulWorker* player);


#endif