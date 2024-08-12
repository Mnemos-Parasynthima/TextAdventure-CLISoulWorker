#ifndef _MAZE_
#define _MAZE_

#include <stdbool.h>

#define NO_EXIT 0xFEEDFAED

// A structure representing a room within a maze. Has connections to other possible rooms.
typedef struct Room {                                       // 57B+7B(PAD) = 64B
  struct Room* exits[4]; // The possible exits that a room can have          32B
  char* info; // The description of the room.                                 8B
  // At initialization, the pointers will be the room ids in hex (-1 -> 0xFEEDFAED; 0 -> 0x0; 1 -> 0x1; ...; 10 -> 0xa; etc..)
  char* enemy; // The possible enemy that the room can have                   8B
  char* loot; // The possible loot item that the room can have                8B
  char id; // The room id, note: it is not a string, just a number of 1 byte  1B
} Room;

// A structure representing a single maze with an entry.
typedef struct Maze {                      // 9B+7B(PAD) = 16B
  Room* entry; // The entrance of the maze                  8B  
  // char* name; // The name of the maze [Unused for now]   8B
  char size; // The number of rooms that the maze has       1B
} Maze;

/**
 * Deletes the given room.
 * @param room The room to delete
 */
void deleteRoom(Room* room);

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


#endif