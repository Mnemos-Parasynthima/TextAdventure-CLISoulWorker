CC = gcc
CFLAGS = -Wall

INCLUDES = -I. -Iheaders

SRCS = cJSON.c main.c RoomTable.c Setup.c SoulWorker.c Maze.c Error.c Keyboard.c SaveLoad.c itoa.s DArray.c Misc.c

HEADERS = headers/cJSON.h headers/Setup.h headers/SoulWorker.h headers/Maze.h headers/Error.h  \
		headers/Keyboard.h headers/SaveLoad.h headers/LoadJSON.h headers/DArray.h headers/Misc.h

OBJS = $(SRCS:.c=.o)
OBJS := $(OBJS:.s=.o)

TARGET = clisw

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $< -o $@

debug: CFLAGS += -g -O0
debug: $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all debug clean