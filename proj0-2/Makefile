CC = gcc
CFLAGS = -g

TARGET = testlib

SOURCE = bitmap.c debug.c hash.c hex_dump.c list.c main.c
OBJECTS = bitmap.o debug.o hash.o hex_dump.o list.o main.o
INC = bitmap.h debug.h hash.h hex_dump.h limits.h list.h round.h

$(TARGET) : $(OBJECTS) $(INC)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -rf $(OBJECTS) $(TARGET)
	