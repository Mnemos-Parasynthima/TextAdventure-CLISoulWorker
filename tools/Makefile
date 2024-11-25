CC = gcc

CFLAGS = -Wall -Wextra

PARENT_OBJ = itoa.o error.o cJSON.o

HEADERS = ../headers/Error.h ../headers/Colors.h

TARGETS = room maze item enemy item
EXES = CreateMaze CreateRoom CreateItem CreateEnemy

.PHONY: all clean $(TARGETS)

all: $(TARGETS)

debug: CFLAGS += -g -O0
debug: $(TARGETS)

maze: $(PARENT_OBJ) $(HEADERS) ../headers/cJSON.h
	$(CC) $(CFLAGS) CreateMaze.c $(PARENT_OBJ) -o CreateMaze

room: $(PARENT_OBJ) $(HEADERS)
	$(CC) $(CFLAGS) -I../headers/ CreateRoom.c $(PARENT_OBJ) -o CreateRoom

item: $(PARENT_OBJ) $(HEADERS)
	$(CC) $(CFLAGS) -I../headers/ CreateItem.c $(PARENT_OBJ) -o CreateItem

enemy: $(PARENT_OBJ) $(HEADERS)
	$(CC) $(CFLAGS) -I../headers/ CreateEnemy.c $(PARENT_OBJ) -o CreateEnemy

itoa.o: ../itoa.s
	$(CC) $< -c -o $@

error.o: ../Error.c ../headers/Error.h
	$(CC) $(CFLAGS) -I../headers/ $< -c -o $@

cJSON.o: ../cJSON.c ../headers/cJSON.h
	$(CC) $(CFLAGS) -I../headers/ $< -c -o $@

clean:
	rm -f $(EXES) $(PARENT_OBJ) *.o