CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pedantic -ggdb -I./SDL2/include
LDFLAGS = -L./SDL2/lib -lSDL2 -lm

SRC = main.c la.c buffer.c
OBJ = $(SRC:.c=.o)
EXEC = text_editor

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
