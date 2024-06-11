CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pedantic -ggdb -I./SDL2/include -I./glew/include
LDFLAGS = -L./SDL2/lib -L./glew/lib/Release/x64 -lSDL2 -lglew32 -lopengl32 -lm

SRC = src/main.c src/la.c src/editor.c src/file.c src/gl_extra.c src/sdl_extra.c src/font.c
OBJ = $(SRC:.c=.o)
EXEC = te

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
