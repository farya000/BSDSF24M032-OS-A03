
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

SRC = src/main.c src/shell.c src/execute.c
OBJ = obj/main.o obj/shell.o obj/execute.o

TARGET = bin/psh

all: $(TARGET)

$(TARGET): $(OBJ)
	gcc -o $(TARGET) $(OBJ) -lreadline

obj/%.o: src/%.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/*.o bin/psh

.PHONY: all clean

