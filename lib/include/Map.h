#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1024
#define MAX_WALLS 6

typedef struct MAP{
    int x_min, x_max;
    int y_min, y_max;
    int width, height;
} MAP;

extern MAP walls[MAX_WALLS];
void convertWallsToRects(MAP *walls, SDL_Rect *rects, int count);

#endif