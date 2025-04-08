## Makefile for Windows + SDL2 (MSYS2)

SRCDIR = ./src
CC = gcc

# SDL2 paths – include the parent of SDL2/
INCLUDE = /mingw64/include
LIBS = /mingw64/lib

CFLAGS = -g -I$(INCLUDE) -c
LDFLAGS = -L$(LIBS) -lmingw32 -lSDL2main -lSDL2 -mwindows -lSDL2_image -lSDL2_ttf -lSDL2_mixer

OBJ = main.o
EXE = main.exe

$(EXE): $(OBJ)
	@echo "Linking $(EXE)"
	$(CC) $(OBJ) -o $(EXE) $(LDFLAGS)

$(OBJ): $(SRCDIR)/main.c
	@echo "Compiling $<"
	$(CC) $(CFLAGS) $< -o $@

clean:
	del $(EXE)
	rm -f $(EXE) $(OBJ)