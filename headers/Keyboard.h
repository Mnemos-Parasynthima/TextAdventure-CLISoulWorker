#include <stdbool.h>
#include <stdio.h>

#include "Setup.h"
#include "SoulWorker.h"


typedef enum {
  WALK = 'm',
  OPEN_INVENTORY = 'i',
  HELP = 'h'
} Commands;

/**
 * Checks whether the given action is a valid action.
 * @param action The action to check
 * @return True if a valid action, false otherwise
 */
bool performAction(Commands action, SoulWorker* player);