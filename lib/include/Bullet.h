#ifndef BULLET_H
#define BULLET_H

#include <SDL2/SDL.h>
#include "Map.h"


#define BULLET_SPEED 6.0f
#define BULLETLIFETIME 3000

typedef struct {
    int whoShot;
    float x, y;
    float dx, dy;
    Uint32 bornTime;
    SDL_Texture* texture;
} Bullet;

Bullet* createBullet(SDL_Renderer* renderer, float startX, float startY, float dirX, float dirY, int whoShot);
void moveBullet(Bullet* bullet);
void drawBullet(Bullet* bullet, SDL_Renderer* renderer);
void destroyBullet(Bullet* bullet);
int checkCollisionBulletWall(Bullet* bullet, MAP* walls, int numWalls);

#endif