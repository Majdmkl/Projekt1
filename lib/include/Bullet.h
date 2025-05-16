#ifndef BULLET_H
#define BULLET_H

#include <SDL2/SDL.h>
#include "Map.h"
#include "config.h"

typedef struct Bullet {
  int whoShot;
  Uint32 bornTime;
  float x, y, dx, dy;
  SDL_Texture* texture;
} Bullet;

float xBullet(Bullet* bullet);
float yBullet(Bullet* bullet);
float DxBullet(Bullet* bullet);
float DyBullet(Bullet* bullet);
void moveBullet(Bullet* bullet);
void destroyBullet(Bullet* bullet);
SDL_Rect getBulletRect(Bullet* bullet);
Uint32 getBulletBornTime(Bullet* bullet);
void drawBullet(Bullet* bullet, SDL_Renderer* renderer);
int checkCollisionBulletWall(Bullet* bullet, MAP* walls, int numWalls);
Bullet* createBullet(SDL_Renderer* renderer, float startX, float startY, float dirX, float dirY, int whoShot);

#endif