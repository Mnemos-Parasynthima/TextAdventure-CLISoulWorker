#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

#ifdef _WIN64
  #include "./headers/dirent.h"
  #include "./headers/getopt.h"
  #include <direct.h> // replace sys/stat
  #include <process.h> // replace sys/wait
#else
  #include <dirent.h>
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
#endif



#define BAD_EXIT 0xff // -1

#ifdef _WIN64
  #define INSTALLER "clisw-installer.exe"
  #define LAUNCHER "clisw-launcher.exe"
  #define GAME "clisw.exe"
#else
  #define INSTALLER "clisw-installer"
  #define LAUNCHER "clisw-launcher"
  #define GAME "clisw"
#endif

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
 * Wrapper around sleep function for Windows and Linux.
 * @param ms Amount of milliseconds to sleep for
 */
void ssleep(int ms) {
  #ifdef _WIN64
    Sleep(ms);
  #else
    usleep(ms * 1000);
  #endif
}

/**
 * Creates a bash script that cross-checks the current version with the hosted verion, indicating whether to update or not.
 * @return True if game is latest, false otherwise
 */
bool checkLatest() {
  const char* versionScript;
  int scriptLen;

#ifdef _WIN64
  scriptLen = 11;
  char* scriptContents[] = {
    // "for /f "tokens=*" %%i in ('curl -s https://raw.githubusercontent.com/yourusername/yourgame/main/version.txt') do set LATEST_VERSION=%%i",
    "@echo off\n",
    "set LATEST_VERSION=alpha-3.0.0\n",
    "set /p CURRENT_VERSION=<version\n",
    "echo Checking version...\n",
    "echo Current version is %CURRENT_VERSION%\n",
    "echo Latest version is %LATEST_VERSION%\n",
    "if '%LATEST_VERSION%' neq '%CURRENT_VERSION%' (\n",
    "  exit /b 64\n",
    ") else (\n",
    "  exit /b 128\n",
    ")"
  };

  versionScript = "check.bat";
#else
  scriptLen = 9;
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

  versionScript = "check.sh";
#endif

  FILE* tmp = fopen(versionScript, "w");
  if (!tmp) {
    printf("Could not create temp check file!\n");
    exit(BAD_EXIT);
  }

  for (int i = 0; i < scriptLen; i++) {
    fwrite(scriptContents[i], sizeof(char), strlen(scriptContents[i]), tmp);
  }

  fclose(tmp);

  int status, exitStatus;

#ifdef _WIN64
  status = _spawnlp(_P_WAIT, versionScript, versionScript, NULL);

  if (status == -1) {
    perror("ERROR");
    printf("Could not execute version checker!\n");
    exit(BAD_EXIT);
  }

  exitStatus = status;
#else
  system("chmod +x check.sh");

  pid_t pid = fork();
  status = 0;

  if (pid == -1) {}

  if (!pid) { // child
    if (execl(versionScript, versionScript, (char*) NULL) == -1) {
      perror("ERROR");
      printf("Could not execute version checker!\n");
      exit(BAD_EXIT);
    }
  }

  wait(&status);
  exitStatus = WEXITSTATUS(status);
#endif

  int code = exitStatus >> 6; 

  if (code == 1) {// versions do not match
    return false;
  } else if (code == 2) { // versions match
    return true;
  } else {
    printf("Version checker terminated abnormally!\n");
    exit(-1);
  }
  
  return false;
}

/**
 * Runs the installer based on the passed options.
 * @param opt The option for the installer
 */
void runInstaller(enum OPTS opt) {
  printf("Changing to directory...\n");
  ssleep(1000);

  int ret;
#ifdef _WIN64
  ret = _chdir(INSTALLER_DIR);
#else
  ret = chdir(INSTALLER_DIR);
#endif

  if (ret == -1) {
    perror("ERROR");
    printf("Cannot change into directory. Exiting...\n");
    exit(-1);
  }

  char* args[3] = {INSTALLER, NULL, NULL};

  if (opt == FIX) args[1] = "-f";
  else args[1] = "-u";

  int execRet;
#ifdef _WIN64
  execRet = _spawnv(_P_OVERLAY, INSTALLER, args);
#else
  execRet = execv(INSTALLER, args);
#endif

  if (execRet == -1) {
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

  ssleep(1000);
  
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
  ssleep(1000);

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

  int ret;

#ifdef __linux__
  mode_t flags = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
#endif

  DIR* saves = opendir(SAVES_DIR);
  if (!saves) {
    printf("Could not find save data! Creating saves...\n");

#ifdef _WIN64
    ret = _mkdir(SAVES_DIR);
#else
    ret = mkdir(SAVES_DIR, flags);
#endif

    if (ret == -1) {
      if (errno != EEXIST) {
        perror("ERROR");
        printf("Cannot create saves directory. Exiting...\n");
        exit(-1);
      }
    }

#ifdef _WIN64
    ret = _mkdir(SAVES_MAPS_DIR);
#else
    ret = mkdir(SAVES_MAPS_DIR, flags);
#endif
    if (ret == -1) {
      if (errno != EEXIST) {
        perror("ERROR");
        printf("Cannot create save maps directory. Exiting...\n");
        exit(-1);
      }
    }
  }
  closedir(saves);

  DIR* savesMaps = opendir(SAVES_MAPS_DIR);
  if (!savesMaps) {
    printf("Could not find save map data! Creating save maps...\n");

#ifdef _WIN64
    ret = _mkdir(SAVES_MAPS_DIR);
#else
    ret = mkdir(SAVES_MAPS_DIR, flags);
#endif
    if (ret == -1) {
      if (errno != EEXIST) {
        perror("ERROR");
        printf("Cannot create save maps directory. Exiting...\n");
        exit(-1);
      }
    }
  }
  closedir(savesMaps);

  printf("Verification done. Game starting...\n");
  ssleep(1000);

  int execRet;
  const char* launcherFlag = "-l";
#ifdef _WIN64
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  char* argvIn[13] = {0};
  snprintf(argvIn, 13, "%s %s", GAME, launcherFlag);
  execRet = (int) CreateProcess(GAME, argvIn, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
  if (!execRet) {
    printf("COULD NOT EXECUTE CLISW.EXE, ABORTING\n");
    exit(-1);
  }

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
#else
  execRet = execl(GAME, GAME, launcherFlag, NULL);
  if (execRet == -1) {
    printf("COULD NOT EXECUTE CLISW, ABORTING\n");
    exit(-1);
  }
#endif
  return 0;
}