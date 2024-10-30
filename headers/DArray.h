#include <stdbool.h>

#ifndef _BYTE_
#define byte unsigned char
#endif

typedef struct DArray {
  char* ids; // The array containing room ids
  byte size; // The size of the array
} DArray;


/**
 * Initiates the dynamic array with the given size.
 * @param size The size for the array.
 * @return The dynamic array
 */
DArray* initDArray(byte size);

/**
 * Adds an id to the given array.
 * @param arr The array to add to
 * @param id The id to add
 */
void dArrayAdd(DArray* arr, byte id);

/**
 * Checks whether the given id exists in the array.
 * @param arr The array to check
 * @param id The target id
 * @return True if if exists, false otherwise
 */
bool dArrayExists(DArray* arr, byte id);

/**
 * Frees the array and nulls the pointer.
 * @param arr The array to free
 */
void dArrayFree(DArray* arr);