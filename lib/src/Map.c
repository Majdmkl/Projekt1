#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Map.h"
#include "Character.h"

MAP walls[] = {
    {0, SCREEN_WIDTH, 0, 0},
    {0, 0, 0, SCREEN_HEIGHT},
    {SCREEN_WIDTH, SCREEN_WIDTH, 0, SCREEN_HEIGHT},
    {0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_HEIGHT},

    {300, 428, 300, 428}, // Tree obstacle
    {600, 745, 600, 730}  // Cottage obstacle
};

void convertWallsToRects(MAP* walls, SDL_Rect* rects, int count) {
    for (int i = 0; i < count; i++) {
        rects[i].x = walls[i].x_min;
        rects[i].y = walls[i].y_min;
        rects[i].w = walls[i].x_max - walls[i].x_min;
        rects[i].h = walls[i].y_max - walls[i].y_min;
    }
}