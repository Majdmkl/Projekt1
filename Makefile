CC = gcc
CFLAGS = -Wall -g -I/opt/homebrew/include/SDL2
LDFLAGS = -L/opt/homebrew/lib -lSDL2 -lSDL2_image
SRC = src/main.c
OUT = build/main

all:
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

run:
	./$(OUT)

clean:
	rm -rf build
