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


MAP* createMap(SDL_Renderer* renderer) {
    MAP* map = (MAP*)malloc(sizeof(MAP));
    if (!map) { SDL_Log("Failed to allocate memory for map"); return NULL; }

    map->width = NR_Y;
    map->height = NR_X;

    map->tileTextures = (SDL_Texture**)malloc(2 * sizeof(SDL_Texture*));
    if (!map->tileTextures) { free(map); return NULL; }

    SDL_Surface* surface = IMG_Load("lib/assets/grass.png");
    if (!surface) {
        SDL_Log("Failed to load grass texture: %s", IMG_GetError());
        free(map->tileTextures);
        free(map);
        return NULL;
    }

    map->tileTextures[0] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("lib/assets/water.png");
    if (!surface) {
        SDL_Log("Failed to load water texture: %s", IMG_GetError());
        SDL_DestroyTexture(map->tileTextures[0]);
        free(map->tileTextures);
        free(map);
        return NULL;
    }

    map->tileTextures[1] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("lib/assets/objects/Tree.png");
    if (!surface) {
        SDL_Log("Failed to load tree texture: %s", IMG_GetError());
        SDL_DestroyTexture(map->tileTextures[0]);
        SDL_DestroyTexture(map->tileTextures[1]);
        free(map->tileTextures);
        free(map);
        return NULL;
    }
    map->treeTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("lib/assets/objects/Cottage.png");
    if (!surface) {
        SDL_Log("Failed to load cottage texture: %s", IMG_GetError());
        SDL_DestroyTexture(map->tileTextures[0]);
        SDL_DestroyTexture(map->tileTextures[1]);
        SDL_DestroyTexture(map->treeTexture);
        free(map->tileTextures);
        free(map);
        return NULL;
    }
    map->cottageTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    return map;
}

void renderMap(MAP* map, SDL_Renderer* renderer) {
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            SDL_Rect dst = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
            SDL_RenderCopy(renderer, map->tileTextures[mapData[y][x]], NULL, &dst);
        }
    }

    SDL_Rect treeRect = { 300, 300, 128, 128 };
    SDL_Rect cottageRect = { 600, 600, 145, 130 };
    SDL_RenderCopy(renderer, map->treeTexture, NULL, &treeRect);
    SDL_RenderCopy(renderer, map->cottageTexture, NULL, &cottageRect);
}

bool isValidPosition(float x, float y)
{
    int tileX = x / TILE_SIZE;
    int tileY = y / TILE_SIZE;

    if (tileX < 0 || tileX >= 14 || tileY < 0 || tileY >= 8) return false;
    if (mapData[tileY][tileX] == 1) return false;

    for (int i = 0; i < 23; i++)
        if (x + CHARACTER_WIDTH > walls[i].x_min && x < walls[i].x_max && y + CHARACTER_HEIGHT > walls[i].y_min && y < walls[i].y_max) return false;

    return true;
}

void destroyMap(MAP* map) {
    if (map) {
        if (map->tileTextures) {
            SDL_DestroyTexture(map->tileTextures[0]);
            SDL_DestroyTexture(map->tileTextures[1]);
            free(map->tileTextures);
        }
        if (map->treeTexture) SDL_DestroyTexture(map->treeTexture);
        if (map->cottageTexture) SDL_DestroyTexture(map->cottageTexture);
        free(map);
    }
}

void convertWallsToRects(MAP* walls, SDL_Rect* rects, int count) {
    for (int i = 0; i < count; i++) {
        rects[i].x = walls[i].x_min;
        rects[i].y = walls[i].y_min;
        rects[i].w = walls[i].x_max - walls[i].x_min;
        rects[i].h = walls[i].y_max - walls[i].y_min;
    }
}