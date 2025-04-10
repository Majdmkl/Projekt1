#ifndef WORLD_H
#define WORLD_H

#include "../include/Character.h"
#include "../include/Bullet.h"

#define MAP_WIDTH 1024
#define MAP_HEIGHT 1024
#define PLAYABLE_AREA_X_MIN 38
#define PLAYABLE_AREA_X_MAX 760
#define PLAYABLE_AREA_Y_MIN 55
#define PLAYABLE_AREA_Y_MAX 763

…
typedef struct {
    int x_min;
    int x_max;
    int y_min;
    int y_max;
} Wall;

//make struct accesible to other files. (can be modified as a pointed for easier reach later)
extern Wall walls[23];

void convertWallsToRects(Wall *walls, SDL_Rect *rects, int count);

#endif