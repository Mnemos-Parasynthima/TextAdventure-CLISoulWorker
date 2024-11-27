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
  #include <sys/types.h>
  #include <sys/stat.h>
#endif

#define PACKAGE "clisw_build.zip"
#define PACKAGE_SIZE 15

#define GAME_DIR "CLISW_GAME"
#define SAVES_DIR "data/saves"


enum OPTS {
  FIX,
  UPDATE,
  INSTALL,
  NONE
};

/**
 * Wrapper around sleep function for Windows and Linux.
 * @param ms Amount of milliseconds to sleep for
 */
static void ssleep(int ms) {
  #ifdef _WIN64
    Sleep(ms);
  #else
    usleep(ms * 1000);
  #endif
}

/**
 * Parses the arguments.
 * @param argc 
 * @param argv 
 * @return 
 */
static enum OPTS parseArgs(int argc, char const* argv[]) {
  int c;
  enum OPTS opt = NONE;

  while ((c = getopt(argc, (char * const*) argv, "fui")) != -1) {
    switch (c) {
      case 'f':
        opt = FIX;
        break;
      case 'u':
        opt = UPDATE;
        break;
      case 'i':
        opt = INSTALL;
        break;
      default:
#ifdef _WIN64
        printf("usage: clisw-installer.exe [-f|-u|-i]\n");
#else
        printf("usage: clisw-installer [-f|-u|-i]\n");
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
      else opt = INSTALL;
    }
  } 
  
  if (opt == FIX) printf("Fixing file integrity...\n");
  else if (opt == UPDATE) printf("Updating...\n");
  else printf("Installing game...\n");

  return opt;
}


int main(int argc, char const* argv[]) {
  enum OPTS opt = parseArgs(argc, argv);

  printf("Creating game directory...\n");
  ssleep(1000);
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
  ssleep(1000);
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
  ssleep(1000);

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

  if (opt == FIX) printf("Fixing game...\n"); // overwrite everything except for the save files
  else if (opt == UPDATE) {
    // for updating the game, there is a chance the save format is updated
    // so updating save format (??????)
    printf("Updating game...\n");
  } else printf("Installing game...\n"); // fresh install
  ssleep(1000);

#ifdef _WIN64
    zipCmdSize = 9 + PACKAGE_SIZE;
    zipCmd = "tar -xf %s";
#else
    zipCmdSize = 7 + PACKAGE_SIZE;
    zipCmd = "unzip %s";
#endif

  char* cmd = (char*) malloc(sizeof(char) * zipCmdSize);
  if (!cmd) {
    printf("COULD NOT ALLOCATE MEMORY!\n");
    exit(-1);
  }

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
  // del somehow not deleting zip file, future me issue
  endCmd = "robocopy CLISW . /move /e && del /q \"%s\"";
#else
  endCmdLen = 45;
  endCmd = "rsync -a CLISW/ ./ && rm -rf CLISW && rm -f %s";
#endif

  char* temp = realloc(cmd, endCmdLen + PACKAGE_SIZE);
  if (!temp) {
    printf("COULD NOT ALLOCATE MEMORY!\n");
    exit(-1);
  }
  cmd = temp;

  snprintf(cmd, endCmdLen + PACKAGE_SIZE, endCmd, PACKAGE);

  sysret = system(cmd);
  free(cmd);
  if (sysret == -1) {
    perror("ERROR FOR MOVE AND REMOVE");
    printf("Could not move data out and delete. Exiting...\n");
    exit(-1);
  }

  // Now that everything is installed and `data` is created, create the save and saved maps directories, only when fresh install
  // when wanting to update, keep saves intact
  // when wanting to fix files (aka no file(s) found), keep saves intact
  if (opt == INSTALL) {
    printf("Creating saves directory...\n");
    ssleep(1000);
#ifdef _WIN64
    ret = _mkdir(SAVES_DIR);
#else
    ret = mkdir(SAVES_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif

    if (ret == -1) {
      if (errno != EEXIST) {
        perror("ERROR");
        printf("Cannot create saves directory. Exiting...\n");
        exit(-1);
      }
    }

    printf("Installing complete!\n");
  } else if (opt == FIX) printf("Fix complete!\n");
  else printf("Update complete!\n");
  ssleep(1000);

  return 0;
}