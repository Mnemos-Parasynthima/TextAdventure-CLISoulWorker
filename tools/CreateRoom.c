#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "Error.h"
#include "Colors.h"

#define UNUSED __attribute__ ((unused))

/**
 * Converts integer n into a character string.
 * @param n The integer
 * @param buffer The buffer to hold the string
 * @return The integer as a string
 */
extern str itoa(int n, char* buffer, int radix);

#define FLUSH() do { int c; while ((c = getchar()) != '\n' && c != EOF); } while (0);


int main(int argc UNUSED, char const* argv[] UNUSED) {
  int rooms, id = 0;

  printf("How many rooms? ");
  fscanf(stdin, "%d", &rooms);
  FLUSH()

  FILE* room;
  str filename = NULL;
  char buffer[3];
  str line = NULL;
  size_t n;

  while (id != rooms) {
    filename = (str) malloc(18 + strlen(itoa(id, buffer, 10)));
    sprintf(filename, "./out/rooms/%d.room", id);
    room = fopen(filename, "w");
    if (!room) handleError(ERR_IO, FATAL, "Could not create room %d file!\n", id);

    printf("For room %d:\n", id);

    fprintf(room, "%d\n", id);
    printf("\t%sSet room id to %d\n%s", GREEN, id, RESET);

    bool boss = false;
    printf("\tIs boss room? [y|n]: ");
    char c = getchar();
    FLUSH()
    if (c == 'y') boss = true;

    str storyfile;
    if (id == 0) storyfile = "entry.story\n";
    else if (boss) storyfile = "boss.story\n";
    else storyfile = "\n";

    fputs(storyfile, room);
    printf("\t%sSet storyfile to %s%s", GREEN, (*storyfile == '\n') ? "[none]\n" : storyfile, RESET);

    printf("\tEnter info file: ");
    getline(&line, &n, stdin);
    fputs(line, room);
    printf("\t%sSet info file to %s%s", GREEN, line, RESET);

    printf("\tEnter exits: ");
    getline(&line, &n, stdin);
    fputs(line, room);
    printf("\t%sSet exits to %s%s", GREEN, line, RESET);

    fputs((boss) ? "1\n" : "0\n", room);
    printf("\t%sSet boss to %s\n%s", GREEN, (boss) ? "1" : "0", RESET);

    printf("\tEnter loot (-1 for none or type::id,...): ");
    getline(&line, &n, stdin);
    fputs(line, room);
    printf("\t%sSet loot to %s%s", GREEN, line, RESET);

    printf("\tEnter enemies (-1 for none or id,...): ");
    getline(&line, &n, stdin);
    fputs(line, room);
    printf("\t%sSet enemies to %s%s", GREEN, line, RESET);

    id++;

    printf("Room created and saved to %s!\n", filename);

    fclose(room);
    free(filename);
  }

  return 0;
}