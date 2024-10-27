#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


#define GAME_DIR "CLISW_GAME"
#define PACKAGE "clisw_build.zip"
#define PACKAGE_SIZE 15


enum OPTS {
  FIX,
  UPDATE,
  NONE
};

/**
 * Parses the arguments.
 * @param argc 
 * @param argv 
 * @return 
 */
enum OPTS parseArgs(int argc, char const *argv[]) {
  int c;
  enum OPTS opt = NONE;

  while ((c = getopt(argc, (char * const*) argv, "f")) != -1) {
    switch (c) {
      case 'f':
        opt = FIX;
        break;
      case 'u':
        opt = UPDATE;
      default:
        printf("usage: clisw-installer [-f|-u]");
        break;
    }
  }

  // In the case the program is ran w/o args, ask user for option
  // Since it is ran my click, the argument is not present
  if (opt == NONE) {
    printf("Do you want to fix file integrity? (Note: this keeps your saves intact) [y|n] ");

    char res = tolower(getchar());
    int c; while ((c = getchar()) != '\n' && c != EOF);
    
    if (res == 'y') opt = FIX;
    else {
      printf("Do you want to update the game? [y|n] ");

      res = tolower(getchar());
      if (res == 'y') opt = UPDATE;
    }
  } 
  
  if (opt == FIX) printf("Fixing file integrity...\n");
  else if (opt == UPDATE) printf("Updating...\n");
  else printf("Installing game...\n");

  return opt;
}


int main(int argc, char const *argv[]) {
  enum OPTS opt = parseArgs(argc, argv);

  if (mkdir(GAME_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
    if (errno != EEXIST) {
      perror("ERROR");
      printf("Cannot create directory. Exiting...\n");
      exit(-1);
    } else printf("Directory exists!\n");
  }

  if (chdir(GAME_DIR) == -1) {
    perror("ERROR");
    printf("Cannot change into directory. Exiting...\n");
    exit(-1);
  }

  // download (cp for now)
  system("cp ../clisw_build.zip .");

  // unzip the contents
  int zipCmdSize;
  char* zipCmd = NULL;
  if (opt == FIX || opt == UPDATE) { // update is temp for now
    // overwrite everything except for the save files
    zipCmdSize = 29 + PACKAGE_SIZE;
    zipCmd = "unzip %s -x CLISW/data/saves/*";

    // note, it will overwrite launcher
    // in the case that the installer was called by 
  } else if (opt == UPDATE) {
    // for updating the game, there is a chance the save format is updated
    // so updating save format (??????)
  } else {
    // fresh install
    zipCmdSize = 7 + PACKAGE_SIZE;
    zipCmd = "unzip %s";
  }
  char* cmd = (char*) malloc(sizeof(char) * zipCmdSize);
  snprintf(cmd, zipCmdSize, zipCmd, PACKAGE);
  system(cmd);
  // if (!system(cmd)) {
  //   perror("ERROR");
  //   printf("Could not unzip. Exiting...\n");
  //   exit(-1);
  // }
  // int ret = system(cmd);

  // 
  cmd = realloc(cmd, 40 + PACKAGE_SIZE);
  // NOTE: CLISW is the directory made at zip time
  snprintf(cmd, 40 + PACKAGE_SIZE, "mv CLISW/* ./ && rm -rf CLISW && rm -f %s", PACKAGE);
  system(cmd);

  free(cmd);

  printf("Installing complete!\n");

  return 0;
}
