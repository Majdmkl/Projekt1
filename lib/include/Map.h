#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1024
#define PLAYABLE_AREA_X_MIN 38
#define PLAYABLE_AREA_X_MAX 760
#define PLAYABLE_AREA_Y_MIN 55
#define PLAYABLE_AREA_Y_MAX 763

typedef struct MAP{
    int x_min, x_max;
    int y_min, y_max;
    int width, height;
    SDL_Texture** tileTextures;
    SDL_Texture* treeTexture;
    SDL_Texture* cottageTexture;
} MAP;

extern MAP walls[23];

void destroyMap(MAP* map);
MAP* createMap(SDL_Renderer* renderer);
void renderMap(MAP* map, SDL_Renderer* renderer);
void convertWallsToRects(MAP *walls, SDL_Rect *rects, int count);

#endif