#include <stdlib.h>

#include "DArray.h"
#include "Error.h"

DArray* initDArray(char size) {
  DArray* arr = (DArray*) malloc(sizeof(DArray));
  if (arr == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for the dynamic array structure!\n");

  arr->ids = (char*) malloc(size);
  if (arr->ids == NULL) handleError(ERR_MEM, FATAL, "Could not allocate space for the dyanmic array contents!\n");

  arr->size = size;

  for (int i = 0; i < arr->size; i++) arr->ids[i] = -1;

  return arr;
}

void dArrayAdd(DArray* arr, char id) {
  if (arr == NULL) return;

  for (int i = 0; i < arr->size; i++) {
    if (arr->ids[i] == -1) {
      arr->ids[i] = id;
      return;
    }
  }
}

bool dArrayExists(DArray* arr, char id) {
  if (arr == NULL) return false;

  for (int i = 0; i < arr->size; i++) {
    if (arr->ids[i] == id) return true;
  }

  return false;
}

void dArrayFree(DArray* arr) {
  free(arr->ids);
  free(arr);
  arr = NULL;
}