CC = gcc

CFLAGS = -Wall -Wextra

PARENT_OBJ = itoa.o error.o cJSON.o

HEADERS = ../headers/Error.h ../headers/cJSON.h

TARGETS = room maze item enemy
EXES = CreateMaze CreateRoom CreateItem CreateEnemy

.PHONY: all clean $(TARGETS)

all: $(TARGETS)

maze: $(PARENT_OBJ) $(HEADERS)
	$(CC) $(CFLAGS) CreateMaze.c $(PARENT_OBJ) -o CreateMaze

room: $(PARENT_OBJ) ../headers/Error.h
	$(CC) $(CFLAGS) -I../headers/ CreateRoom.c $(PARENT_OBJ) -o CreateRoom

itoa.o: ../itoa.s
	$(CC) $< -c -o $@

error.o: ../Error.c ../headers/Error.h
	$(CC) $(CFLAGS) -I../headers/ $< -c -o $@

cJSON.o: ../cJSON.c ../headers/cJSON.h
	$(CC) $(CFLAGS) -I../headers/ $< -c -o $@

clean:
	rm -f $(EXES) $(PARENT_OBJ) *.o