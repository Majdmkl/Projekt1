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

<<<<<<< Updated upstream
    {300, 428, 300, 428},
    {600, 745, 600, 730}
=======
    /* {x, x + B, y , y + H} */

    {65, 129, 76, 164} //Träd 1

>>>>>>> Stashed changes
};

static int mapData[NR_X][NR_Y] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

static SDL_Texture* mapBackground = NULL;

MAP* createMap(SDL_Renderer* renderer) {
    MAP* map = (MAP*)malloc(sizeof(MAP));
    if (!map) { SDL_Log("Failed to allocate memory for map"); return NULL; }

    SDL_Surface* surface = IMG_Load("lib/assets/images/objects/MapNew.png");
    if (!surface) {
        SDL_Log("Failed to load map background: %s", IMG_GetError());
        free(map);
        return NULL;
    }

    mapBackground = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    map->width = NR_Y;
    map->height = NR_X;

    return map;
}

void renderMap(MAP* map, SDL_Renderer* renderer) {
    if (mapBackground) {
        SDL_Rect dest = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, mapBackground, NULL, &dest);
    }
}

bool isValidPosition(float x, float y) {
    int tileX = x / TILE_SIZE;
    int tileY = y / TILE_SIZE;

    if (tileX < 0 || tileX >= NR_Y || tileY < 0 || tileY >= NR_X) return false;
    if (mapData[tileY][tileX] == 1) return false;

    for (int i = 0; i < MAX_WALLS; i++)
        if (x + CHARACTER_WIDTH > walls[i].x_min && x < walls[i].x_max &&
            y + CHARACTER_HEIGHT > walls[i].y_min && y < walls[i].y_max) return false;

    return true;
}

void destroyMap(MAP* map) {
    if (mapBackground) SDL_DestroyTexture(mapBackground);
    if (map) free(map);
}

void convertWallsToRects(MAP* walls, SDL_Rect* rects, int count) {
    for (int i = 0; i < count; i++) {
        rects[i].x = walls[i].x_min;
        rects[i].y = walls[i].y_min;
        rects[i].w = walls[i].x_max - walls[i].x_min;
        rects[i].h = walls[i].y_max - walls[i].y_min;
    }
}
