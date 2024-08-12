#ifndef _SETUP_H
#define _SETUP_H


#include <stdbool.h>

#include "Maze.h"


#define FLUSH() int c; while ((c = getchar()) != '\n' && c != EOF);

// A temporary hashmap to store the rooms to later connect them.
typedef struct Table {                              // 16B
  Room** rooms; // The array of room pointers           8B
  int cap; // The current capacity of the table         4B
  int len; // Number of items that the table contains   4B
} Table;


/**
 * Inserts the given room into the given map
 * @param table The map
 * @param room The room to store
 * @param overwrite Whether to overwrite the room if one exists
 * @return Whether the room was put
 */
bool putRoom(Table* table, Room* room, bool overwrite);

/**
 * Creates the maze.
 * @param filename The map to initiate
 * @return The Maze structure
 */
Maze* initMaze(const char* filename);

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

/**
 * Connects the created rooms to form the maze.
 * @param table The map containing the rooms
 * @return The entry room
 */
Room* connectRooms(Table* table);


#endif