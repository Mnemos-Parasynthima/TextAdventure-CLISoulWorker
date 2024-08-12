#include <stdbool.h>

typedef struct DArray {
  char* ids; // The array containing room ids
  char size; // The size of the array
} DArray;


/**
 * Initiates the dynamic array with the given size.
 * @param size The size for the array.
 * @return The dynamic array
 */
DArray* initDArray(char size);

/**
 * Adds an id to the given array.
 * @param arr The array to add to
 * @param id The id to add
 */
void dArrayAdd(DArray* arr, char id);

/**
 * Checks whether the given id exists in the array.
 * @param arr The array to check
 * @param id The target id
 * @return True if if exists, false otherwise
 */
bool dArrayExists(DArray* arr, char id);

/**
 * Frees the array and nulls the pointer.
 * @param arr The array to free
 */
void dArrayFree(DArray* arr);