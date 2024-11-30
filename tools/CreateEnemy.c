#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "Error.h"
#include "Colors.h"


#define UNUSED __attribute__ ((unused))
#define FLUSH() do { int c; while ((c = getchar()) != '\n' && c != EOF); } while (0);

/**
 * Converts integer n into a character string.
 * @param n The integer
 * @param buffer The buffer to hold the string
 * @return The integer as a string
 */
extern str itoa(int n, char* buffer, int radix);

/**
 * Gathers the data for the boss and writes them to the given file.
 * @param file The enemies file
 */
static void getBossData(FILE* file) {
	str line = NULL;
	size_t n;

	printf("\tEnter SoulWeapon drop (soulweapon::[id]): ");
	getline(&line, &n, stdin);
	fputs(line, file);
	printf("\t%sSet SoulWeapon drop to %s%s", GREEN, line, RESET);

	printf("\tEnter helmet drop (helmet::[id]): ");
	getline(&line, &n, stdin);
	fputs(line, file);
	printf("\t%sSet helmet drop to %s%s", GREEN, line, RESET);

	printf("\tEnter shoulder guard drop (shoulder_guard::[id]): ");
	getline(&line, &n, stdin);
	fputs(line, file);
	printf("\t%sSet shoulder guard drop to %s%s", GREEN, line, RESET);

	printf("\tEnter chestplate drop (chestplate::[id]): ");
	getline(&line, &n, stdin);
	fputs(line, file);
	printf("\t%sSet chestplate drop to %s%s", GREEN, line, RESET);

	printf("\tEnter boots drop (boots::[id]): ");
	getline(&line, &n, stdin);
	fputs(line, file);
	printf("\t%sSet boots drop to %s%s", GREEN, line, RESET);

	fputs("START_SKILLS\n", file);

	for (int i = 1; i <= 5; i++) {
		printf("\tFor skill %d:\n", i);

		printf("\tEnter skill name: ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet skill name to %s%s", GREEN, line, RESET);

		printf("\tEnter skill description: ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet skill description to %s%s", GREEN, line, RESET);

		printf("\tEnter skill cooldown: ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet skill cooldown to %s%s", GREEN, line, RESET);

		fprintf(file, "%d\n", i);
		printf("\t%sSet skill ID to %d%s\n", GREEN, i, RESET);

		printf("\tEnter active effect 1 (atk: 0, atk crit dmg: 1): ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet skill active effect 1 to %s%s", GREEN, line, RESET);
		
		int effect = atoi(line);
		if (effect < 0 || effect > 1) handleError(ERR_DATA, FATAL, "Input is not for active effect 1!\n");

		printf("\tEnter active effect 2 (def: 2, acc: 3, atk crit: 4): ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet skill active effect 2 to %s%s", GREEN, line, RESET);
		
		effect = atoi(line);
		if (effect < 2 || effect > 4) handleError(ERR_DATA, FATAL, "Input it not for active effect 2!\n");

		int stat;
		float statf;

		printf("\tEnter effect 1: ");
		fscanf(stdin, "%d", &stat);
		fprintf(file, "%d\n", stat);
		printf("\t%sSet skill effect 1 to %d%s\n", GREEN, stat, RESET);

		printf("\tEnter effect 2: ");
		fscanf(stdin, "%f", &statf);
		fprintf(file, "%3.2f\n", statf);
		printf("\t%sSet skill effect 1 to %d%s\n", GREEN, stat, RESET);

		fputs("END_SKILL\n", file);
		FLUSH()
	}

	if (line != NULL) free(line);
}


int main(int argc UNUSED, char const *argv[] UNUSED) {
	str filename = "./out/enemies.enemy";
	FILE* file = fopen(filename, "w");
	if (!file) handleError(ERR_IO, FATAL, "Could not create file!\n");

	int num;

	printf("How many enemies? ");
	fscanf(stdin, "%d", &num);
	FLUSH()

	int id = 0;

	str line = NULL;
	size_t n;

	while (id != num) {
		printf("For enemy %d:\n", id);

		fprintf(file, "%d\n", id);
		printf("\t%sSet enemy id to %d%s\n", GREEN, id, RESET);

		bool boss = false;

		printf("\tIs boss? [y|n]: ");
		char c = getchar();
    FLUSH()
    if (c == 'y') boss = true;

		printf("\tEnter name: ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet name to %s%s", GREEN, line, RESET);

		printf("\tEnter XP points: ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet XP points to %s%s", GREEN, line, RESET);

		printf("\tEnter HP range (#,#): ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet HP range to %s%s", GREEN, line, RESET);

		printf("\tEnter level: ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet level to %s%s", GREEN, line, RESET);

		printf("\tEnter attack range: ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet attack range to %s%s", GREEN, line, RESET);

		printf("\tEnter accuracy range: ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet accuracy range to %s%s", GREEN, line, RESET);

		printf("\tEnter attack crit range: ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet attack crit range to %s%s", GREEN, line, RESET);

		printf("\tEnter attack crit damage range: ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet attack crit damage range to %s%s", GREEN, line, RESET);

		printf("\tEnter def range: ");
		getline(&line, &n, stdin);
		fputs(line, file);
		printf("\t%sSet def range to %s%s", GREEN, line, RESET);

		if (boss) getBossData(file);

		fputs("END_ENEMY\n", file);
		printf("Enemy created!\n");;

		id++;
	}

	printf("Enemies created and saved to %s!\n", filename);
	fclose(file);

	return 0;
}