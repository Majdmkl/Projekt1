#include <stdio.h>
#include <SDL2/SDL.h>

#include "Map.h"

MAP walls[] = {
    {0, SCREEN_WIDTH, 0, 0},
    {0, 0, 0, SCREEN_HEIGHT},
    {SCREEN_WIDTH, SCREEN_WIDTH, 0, SCREEN_HEIGHT},
    {0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_HEIGHT},


};

void convertWallsToRects(MAP* walls, SDL_Rect* rects, int count) {
    for (int i = 0; i < count; i++) {
        rects[i].x = walls[i].x_min;
        rects[i].y = walls[i].y_min;
        rects[i].w = walls[i].x_max - walls[i].x_min;
        rects[i].h = walls[i].y_max - walls[i].y_min;
    }
}
