## Makefile for Windows + SDL2 (MSYS2)

SRCDIR = ./src
CC = gcc

# SDL2 paths – include the parent of SDL2/
INCLUDE = /mingw64/include
LIBS = /mingw64/lib

CFLAGS = -g -I$(INCLUDE) -c
LDFLAGS = -L$(LIBS) -lmingw32 -lSDL2main -lSDL2 -mwindows

OBJ = hello.o
EXE = hello.exe

$(EXE): $(OBJ)
	@echo "Linking $(EXE)"
	$(CC) $(OBJ) -o $(EXE) $(LDFLAGS)

$(OBJ): $(SRCDIR)/hello_sdl2.c
	@echo "Compiling $<"
	$(CC) $(CFLAGS) $< -o $@

clean:
	del $(EXE)
	del $(OBJ)
