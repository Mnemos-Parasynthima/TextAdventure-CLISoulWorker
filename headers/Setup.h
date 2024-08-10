#ifndef _SETUP_H
#define _SETUP_H


#include <stdbool.h>

#include "Maze.h"


#define FLUSH() int c; while ((c = getchar()) != '\n' && c != EOF);

// A temporary hashmap to store the rooms to later connect them.
typedef struct Table {
  Room** rooms; // The array of room pointers
  int cap; // The current capacity of the table
  int len; // Number of items that the table contains
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