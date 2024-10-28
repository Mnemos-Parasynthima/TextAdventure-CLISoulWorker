#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

#ifdef _WIN64
  #include "./headers/getopt.h"
  #include <direct.h> // replace sys/stat for use of mkdir flags
#else
  #include <unistd.h>
  #include <sys/stat.h>
#endif

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
#ifdef _WIN64
        printf("usage: clisw-installer.exe [-f|-u]");
#else
        printf("usage: clisw-installer [-f|-u]");
#endif
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

  printf("Creating game directory...\n");
  int ret, sysret;

#ifdef _WIN64
  ret = _mkdir(GAME_DIR);
#else
  ret = mkdir(GAME_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
  if (ret == -1) {
    if (errno != EEXIST) {
      perror("ERROR FOR MKDIR");
      printf("Cannot create directory. Exiting...\n");
      exit(-1);
    } else printf("Directory exists!\n");
  }

  printf("Changing to game directory...\n");
#ifdef _WIN64
  ret = _chdir(GAME_DIR);
#else
  ret = chdir(GAME_DIR);
#endif
  if (ret == -1) {
    perror("ERROR FOR CHDIR");
    printf("Cannot change into directory. Exiting...\n");
    exit(-1);
  }

  // download (cp for now)
  printf("Copying zip file...\n");

  char* cpCmd;
#ifdef _WIN64
  cpCmd = "copy \"..\\clisw_build.zip\" .";
#else
  cpCmd = "cp ../clisw_build.zip .";
#endif

  sysret = system(cpCmd);

  if (sysret == -1) {
    perror("ERROR FOR COPY");
    printf("Could not copy. Exiting...\n");
    exit(-1);
  }
  // end download (copy for now)

  // unzip the contents
  int zipCmdSize;
  char* zipCmd = NULL;
  if (opt == FIX || opt == UPDATE) { // update is temp for now
    // overwrite everything except for the save files
    printf("Fixing|Updating game...\n");
  
#ifdef _WIN64
    zipCmdSize = 39 + PACKAGE_SIZE;
    zipCmd = "tar -xf %s -exclude='CLISW/data/saves/*'";
#else
    zipCmdSize = 29 + PACKAGE_SIZE;
    zipCmd = "unzip %s -x CLISW/data/saves/*";
#endif

    // note, it will overwrite launcher
    // in the case that the installer was called by 
  } else if (opt == UPDATE) {
    // for updating the game, there is a chance the save format is updated
    // so updating save format (??????)
  } else {
    // fresh install
    printf("Installing game...\n");
#ifdef _WIN64
    zipCmdSize = 9 + PACKAGE_SIZE;
    zipCmd = "tar -xf %s";
#else
    zipCmdSize = 7 + PACKAGE_SIZE;
    zipCmd = "unzip %s";
#endif
  }
  char* cmd = (char*) malloc(sizeof(char) * zipCmdSize);
  snprintf(cmd, zipCmdSize, zipCmd, PACKAGE);

  sysret = system(cmd);
  if (sysret == -1) {
    perror("ERROR FOR INSTALLING");
    printf("Could not unzip. Exiting...\n");
    exit(-1);
  }

  char* endCmd; // better naming
  size_t endCmdLen;

  // NOTE: CLISW is the directory made at zip time
#ifdef _WIN64
  endCmdLen = 40;
  endCmd = "robocopy CLISW . /move /e && del /q \"%s\"";
#else
  endCmdLen = 40;
  endCmd = "mv CLISW/* ./ && rm -rf CLISW && rm -f %s";
#endif

  cmd = realloc(cmd, endCmdLen + PACKAGE_SIZE);

  snprintf(cmd, endCmdLen + PACKAGE_SIZE, endCmd, PACKAGE);

  sysret = system(cmd);
  if (sysret == -1) {
    perror("ERROR FOR MOVE AND REMOVE");
    printf("Could not move data out and delete. Exiting...\n");
    exit(-1);
  }

  free(cmd);

  printf("Installing complete!\n");

  return 0;
}