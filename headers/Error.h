#ifndef _ERROR_H
#define _ERROR_H


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_RESET         "\x1b[0m"

#ifndef _STR_
#define str char*
#endif

// The type of error encountered
typedef enum {
  ERR_DATA, // Data format error (improper map/save format)
  ERR_MEM, // Unable to allocate memory
  ERR_IO // Cannot open/write file
} errType;

typedef enum {
  FATAL, // Fatal, terminate program
  WARNING // Do not terminate program
} sevType;


/**
 * Handles the error. If the error is fatal, terminates the program.
 * @param err The type of error
 * @param sev The severity of the error
 * @param fmsg The message to print
 */
void handleError(errType err, sevType sev, const str fmsg, ...);


#endif