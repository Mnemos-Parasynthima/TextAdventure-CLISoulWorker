CC = gcc
CFLAGS = -Wall

INCLUDES = -I. -Iheaders

COMMON_SRCS = cJSON.c main.c RoomTable.c Setup.c SoulWorker.c Maze.c
LINUX_SRCS = $(COMMON_SRCS)
WINDOWS_SRCS = $(COMMON_SRCS) getline.c


COMMON_HEADERS = headers/cJSON.h headers/Setup.h headers/SoulWorker.h headers/Maze.h
LINUX_HEADERS = $(COMMON_HEADERS)
WINDOWS_HEADERS = $(COMMON_HEADERS) headers/unistd.h


LINUX_OBJS = $(LINUX_SRCS:.c=.o)
WINDOWS_OBJS = $(WINDOWS_SRCS:.c=.o)

TARGET = clisw

all: $(TARGET)

$(TARGET): $(LINUX_OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(LINUX_OBJS)

%.o: %.c $(LINUX_HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


windows: CC = cl
windows: CFLAGS = /W3 /Zi
windows: INCLUDES = /I. /Iheaders
windows: $(TARGET).exe

$(TARGET).exe: $(WINDOWS_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) /c $< /Fo$@


debug: CFLAGS += -g -O0
debug: $(TARGET)

clean:
	rm -f $(LINUX_OBJS) $(TARGET)
	rm -f $(WINDOWS_OBJS) $(TARGET).exe

.PHONY: all debug clean windows