#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1024
//! same here
#define NR_X 16
#define NR_Y 30
#define TILE_SIZE 64
//! same here

typedef struct MAP{
    int x_min, x_max;
    int y_min, y_max;
    int width, height;
    //! when real map is used remove this
    SDL_Texture** tileTextures;
    SDL_Texture* treeTexture;
    SDL_Texture* cottageTexture;
    //! when real map is used remove this
} MAP;

extern MAP walls[23];
//! 👇
void destroyMap(MAP* map);
MAP* createMap(SDL_Renderer* renderer);
void renderMap(MAP* map, SDL_Renderer* renderer);
//! same for 👆
void convertWallsToRects(MAP *walls, SDL_Rect *rects, int count);

#endif