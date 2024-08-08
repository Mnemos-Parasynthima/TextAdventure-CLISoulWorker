#ifndef _SETUP_
#define _SETUP_

#define NO_EXIT 0xFEEDFAED

// A structure representing a room within a maze. Has connections to other possible rooms.
typedef struct Room {
  char id; // The room id, note: it is not a string, just a number of 1 byte
  char* info; // The description of the room.
  // At initialization, the pointers will be the room ids in hex (-1 -> 0xFEEDFAED; 0 -> 0x0; 1 -> 0x1; ...; 10 -> 0xa; etc..)
  struct Room* exits[4]; // The possible exits that a room can have
  char* enemy; // The possible enemy that the room can have
  char* loot; // The possible loot item that the room can have
} Room;

// A structure representing a single maze with an entry.
typedef struct Maze {
  Room* entry; // The entrance of the maze
  // char* name; // The name of the maze [Unused for now]
  char size; // The number of rooms that the maze has
} Maze;

// A temporary hashmap to store the rooms to later connect them.
typedef struct Table {
  Room** rooms; // The array of room pointers
  int cap; // The current capacity of the table
  int len; // Number of items that the table contains
} Table;

/**
 * Deletes the given room.
 * @param room The room to delete
 */
void deleteRoom(Room* room);

/**
 * Inserts the given room into the given map
 * @param table The map
 * @param room The room to store
 * @return The id of the inserted room
 */
char putRoom(Table* table, Room* room);

/**
 * Creates the maze.
 * @return The Maze structure
 */
Maze* initMaze();

/**
 * Inititates the temporary map to store the room
 * @return The Map
 */
Table* initTable();

/**
 * Deletes the temporary map.
 * @param table The map to delete
 */
void deleteTable(Table* table);

#endif