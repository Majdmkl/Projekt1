#ifndef BULLET_H
#define BULLET_H

#include <SDL2/SDL.h>
#include "Map.h"

#define BULLET_SPEED 10.0f
#define BULLET_LIFETIME 3000

typedef struct Bullet Bullet;

void moveBullet(Bullet* bullet);
void destroyBullet(Bullet* bullet);
Uint32 getBulletBornTime(Bullet* bullet);
void drawBullet(Bullet* bullet, SDL_Renderer* renderer);
int checkCollisionBulletWall(Bullet* bullet, MAP* walls, int numWalls);
Bullet* createBullet(SDL_Renderer* renderer, float startX, float startY, float dirX, float dirY, int whoShot);

#endif