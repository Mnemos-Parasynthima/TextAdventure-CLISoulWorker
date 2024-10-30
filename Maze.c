#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Maze.h"
#include "Error.h"


void removeItemFromMap(Room* room) {
  if (room != NULL && room->loot != NULL) {
    free(room->loot);
    room->loot = NULL;

    return;
  }

  fprintf(stderr, "Could not remove item from map!\n"); // Add to error handling???

  return;
}

bool deleteEnemyFromMap(Room* room, bool deleteGear) {
  if (room == NULL ||  room->enemy.enemy == NULL || room->enemy.boss == NULL) return false;

  bool del;

  if (room->hasBoss) {
    del = deleteBoss(room->enemy.boss, deleteGear);
    room->enemy.boss = NULL;
    room->hasBoss = false;

    return del;
  }
  
  del = deleteEnemy(room->enemy.enemy);
  room->enemy.enemy = NULL;

  return del;
}

static void placeRoomOnGrid(Room* room, char** grid, int x, int y, bool** visited, uint gridSize, Room* playerRoom) {
  if (room == ((void*) ((long long) NO_EXIT)) || visited[y][x]) return;

  visited[y][x] = 1;

  if (room->hasBoss) grid[y][x] = '!';
  else if (room->enemy.enemy != NULL) grid[y][x] = '%';
  else if (room->loot != NULL) grid[y][x] = '$';
  else if (room->id == 0) grid[y][x] = '@';
  else grid[y][x] = '#';

  if (room == playerRoom) grid[y][x] = 'o';


  if (room->exits[0] != ((void*) ((long long) NO_EXIT)) && y > 1) {
    grid[y-1][x] = '|';
    placeRoomOnGrid(room->exits[0], grid, x, y-2, visited, gridSize, playerRoom);
  }
  if (room->exits[1] != ((void*) ((long long) NO_EXIT)) && x < gridSize-2) {
    grid[y][x+1] = '-';
    placeRoomOnGrid(room->exits[1], grid, x+2, y, visited, gridSize, playerRoom);
  }
  if (room->exits[2] != ((void*) ((long long) NO_EXIT)) && y < gridSize-2) {
    grid[y+1][x] = '|';
    placeRoomOnGrid(room->exits[2], grid, x, y+2, visited, gridSize, playerRoom);
  }
  if (room->exits[3] != ((void*) ((long long) NO_EXIT)) && x > 1) {
    grid[y][x-1] = '-';
    placeRoomOnGrid(room->exits[3], grid, x-2, y, visited, gridSize, playerRoom);
  }
}

void showMap(Maze* maze, Room* playerRoom) {
  uint gridSize = (int)(sqrt(maze->size) * 4);
  if (gridSize < 5) gridSize = 5;

  char** grid = malloc(gridSize * sizeof(char*));
  bool** visited = malloc(gridSize * sizeof(bool*));

  
  for (uint i = 0; i < gridSize; i++) {
    grid[i] = malloc(gridSize * sizeof(char));
    visited[i] = malloc(gridSize * sizeof(bool));
    memset(grid[i], ' ', gridSize * sizeof(char));
    memset(visited[i], false, gridSize * sizeof(bool));
  }
  
  placeRoomOnGrid(maze->entry, grid, gridSize/2, gridSize/2, visited, gridSize, playerRoom);
  
  for (int i = 0; i < gridSize; i++) {
    for (int j = 0; j < gridSize; j++) {
      putchar(grid[i][j]);
    }
    putchar('\n');
  }
}

void deleteRoom(Room* room) {
  if (room->loot != NULL) deleteItem(room->loot);
  deleteEnemyFromMap(room, true);
  free(room->info);
  free(room);

  room = NULL;
}

void deleteMaze(Maze* maze) {
  Table* table = initTable();
  if (table == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for the table!\n");

  Room* room = maze->entry;
  addAndRecurse(room, table);
  room = NULL;

  for (int i = 0; i < table->len; i++) {
    deleteRoom(table->rooms[i]);
  }

  deleteTable(table);

  free(maze);
  maze = NULL;
}