CC = gcc
CFLAGS = -Wall

INCLUDES = -I. -Iheaders

SRCS = cJSON.c main.c RoomTable.c Setup.c SoulWorker.c Maze.c

HEADERS = headers/cJSON.h headers/Setup.h headers/SoulWorker.h headers/Maze.h

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