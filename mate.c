#define MATE_IMPLEMENTATION
#include "mate.h"

i32 main() {
  StartBuild();
  {
    Executable exe = CreateExecutable((ExecutableOptions){
      .output = "clisw",
      .flags = "-Wall -lpthread",
      .includes = "-I. -Iheaders"
    });

    char* files[] = {
      "./main.c", "./cJSON.c", "./Setup.c", "./RoomTable.c",
      "./SoulWorker.c", "./Maze.c", "./Error.c", "./Keyboard.c",
      "./SaveLoad.c", "./itoa.s", "./DArray.c", "./Misc.c", "./Battle.c"
    };

    AddFiles(exe, files);
    LinkSystemLibraries(exe, "m");
    InstallExecutable(exe);
  }
  EndBuild();
}
