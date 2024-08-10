CC = gcc
CFLAGS = -Wall

INCLUDES = -I. -Iheaders

SRCS = cJSON.c main.c RoomTable.c Setup.c SoulWorker.c Maze.c Error.c Keyboard.c SaveLoad.c

HEADERS = headers/cJSON.h headers/Setup.h headers/SoulWorker.h headers/Maze.h headers/Error.h headers/Keyboard.h headers/SaveLoad.h

OBJS = $(SRCS:.c=.o)

TARGET = clisw

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


debug: CFLAGS += -g -O0
debug: $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all debug clean