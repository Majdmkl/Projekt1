#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1024
<<<<<<< Updated upstream
#define MAX_WALLS 100

#define NR_X 16
#define NR_Y 30
#define TILE_SIZE 64

=======
#define MAX_WALLS 20
>>>>>>> Stashed changes

typedef struct MAP{
    int x_min, x_max;
    int y_min, y_max;
<<<<<<< Updated upstream
    int width, height;
    SDL_Texture** tileTextures;
    SDL_Texture* treeTexture;
    SDL_Texture* cottageTexture;
=======
>>>>>>> Stashed changes
} MAP;

extern MAP walls[MAX_WALLS];

void loadMap(SDL_Renderer *renderer);
void drawMap(SDL_Renderer *renderer);
void destroyMap(MAP* map);
MAP* createMap(SDL_Renderer* renderer);
void renderMap(MAP* map, SDL_Renderer* renderer);

void convertWallsToRects(MAP *walls, SDL_Rect *rects, int count);

#endif