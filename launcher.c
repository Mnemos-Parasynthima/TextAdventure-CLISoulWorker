#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <errno.h>

#define BAD_EXIT 0xff // -1
#define INSTALLER "clisw-installer"
#define LAUNCHER "clisw-launcher"
#define GAME "clisw"
#define INSTALLER_DIR ".."

#define DATA_DIR "data"
#define MAPS_DIR "data/maps"
#define SAVES_DIR "data/saves"
#define SAVES_MAPS_DIR "data/saves/maps"


#define UNREACHABLE() do { printf("SHOULD NOT REACH THIS!\n"); exit(-1); } while (0);


enum OPTS {
  FIX,
  UPDATE,
  NONE
};

/**
 * Creates a bash script that cross-checks the current version with the hosted verion, indicating whether to update or not.
 * @return True if game is latest, false otherwise
 */
bool checkLatest() {
  char* scriptContents[] = {
    "#!/bin/bash\n",
    "LATEST_VERSION='alpha-3.0.0'\n",
    // "LATEST_VERSION=$(curl -s https://raw.githubusercontent.com/yourusername/yourgame/main/version.txt)\n",
    "CURRENT_VERSION=$(cat version)\n",
    "echo 'Checking version...'\n",
    "if [ $LATEST_VERSION != $CURRENT_VERSION ]; then\n",
    "  exit 64\n",
    "else\n",
    "  exit 128\n",
    "fi"
  };

  FILE* tmp = fopen("check.sh", "w");
  if (!tmp) {
    printf("Could not create temp check file!\n");
    exit(BAD_EXIT);
  }

  for (int i = 0; i < 9; i++) {
    fwrite(scriptContents[i], sizeof(char), strlen(scriptContents[i]), tmp);
  }

  fclose(tmp);

  system("chmod +x check.sh");

  pid_t pid = fork();
  int status = 0;

  if (pid == -1) {}

  if (!pid) { // child
    if (execl("check.sh", "check.sh", (char*) NULL) == -1) {
      perror("ERROR");
      printf("Could not execute version checker!\n");
      exit(BAD_EXIT);
    }
  } else {
    wait(&status);

    int exitStatus = WEXITSTATUS(status);
    int code = exitStatus >> 6; 

    if (code == 1) {// versions do not match
      return false;
    } else if (code == 2) { // versions match
      return true;
    } else {
      printf("Version checker terminated abnormally!\n");
      exit(-1);
    }
  }
  
  return false;
}

/**
 * Runs the installer based on the passed options.
 * @param opt The option for the installer
 */
void runInstaller(enum OPTS opt) {
  if (chdir(INSTALLER_DIR) == -1) {
    perror("ERROR");
    printf("Cannot change into directory. Exiting...\n");
    exit(-1);
  }

  char* args[3] = {INSTALLER, NULL, NULL};

  if (opt == FIX) args[1] = "-f";
  else args[1] = "-u";

  if (execv(INSTALLER, args) == -1) {
    perror("ERROR");
    printf("Could not execute installer!\n");
    exit(BAD_EXIT);
  }
}

int main(int argc, char const* argv[]) {

  // check existance of version file
  FILE* versionFile = fopen("version", "r");
  if (!versionFile) {
    printf("Could not find version file! Fixing files...\n");

    runInstaller(FIX);
    UNREACHABLE()
  }
  fclose(versionFile);

  
  bool latest = checkLatest();
  if (!latest) {
    printf("New version found! Do you want to update? [y|n] \n");
    char response = tolower(getchar());

    if (response == 'y') {
      runInstaller(UPDATE);
      UNREACHABLE()
    } else {
      printf("Will not update. It is highly recommended to update for latest features and fixes!\n");
    }
  }

  // will not update, now verify file integrity
  // for future, do it in a better way
  printf("Verifying files and data...\n");

  FILE* game = fopen(GAME, "r");
  if (!game) {
    printf("Could not find game! Fixing file...\n");
    runInstaller(FIX);
    UNREACHABLE()
  }
  fclose(game);

  FILE* launcher = fopen(LAUNCHER, "r");
  if (!launcher) {
    printf("Could not find launcher! Fixing file...\n");
    runInstaller(FIX);
    UNREACHABLE()
  }
  fclose(launcher);

  DIR* data = opendir(DATA_DIR);
  if (!data) {
    printf("Could not find data! Reinstalling...\n");
    runInstaller(UPDATE);
    UNREACHABLE()
  }
  closedir(data);

  DIR* maps = opendir(MAPS_DIR);
  if (!maps) {
    printf("Could not find map data! Fixing files...\n");
    runInstaller(FIX);
    UNREACHABLE()
  }
  closedir(maps);

  DIR* saves = opendir(SAVES_DIR);
  if (!saves) {
    printf("Could not find save data! Creating saves...\n");

    if (mkdir(SAVES_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
      if (errno != EEXIST) {
        perror("ERROR");
        printf("Cannot create directory. Exiting...\n");
        exit(-1);
      }
    }
    if (mkdir(SAVES_MAPS_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
      if (errno != EEXIST) {
        perror("ERROR");
        printf("Cannot create directory. Exiting...\n");
        exit(-1);
      }
    }
  }
  closedir(saves);

  DIR* savesMaps = opendir(SAVES_MAPS_DIR);
  if (!savesMaps) {
    printf("Could not find save map data! Creating save maps...\n");

    if (mkdir(SAVES_MAPS_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
      if (errno != EEXIST) {
        perror("ERROR");
        printf("Cannot create directory. Exiting...\n");
        exit(-1);
      }
    }
  }
  closedir(savesMaps);

  printf("Verification done. Game starting...\n");

  if (execl(GAME, GAME, "-l", NULL) == -1) {
    printf("COULD NOT EXECUTE CLISW, ABORTING\n");
    exit(-1);
  }

  return 0;
}