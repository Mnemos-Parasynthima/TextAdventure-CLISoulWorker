#ifndef _ERROR_H
#define _ERROR_H


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_RESET         "\x1b[0m"

// The type of error encountered
typedef enum {
  ERR_DATA, // Data format error (improper map/save format)
  ERR_MEM, // Unable to allocate memory
  ERR_IO // Cannot open/write file
} errType;


/**
 * Handles the error, exiting the program.
 * @param err The type of error
 * @param fmsg The message to print
 */
void handleError(errType err, const char* fmsg, ...);


#endif