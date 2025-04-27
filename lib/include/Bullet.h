#ifndef BULLET_H
#define BULLET_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Map.h"
#include "Character.h"

#define BULLET_WIDTH 3
#define BULLET_HEIGHT 3
#define BULLET_SPEED 10
#define BULLETLIFETIME 3000 // 3 seconds

typedef struct {
  int whoShot;
  MAP *walls[23];
  float x, y, dx, dy;
  SDL_Texture *texture;
} Bullet;

float xBullet(Bullet *bullet);
float yBullet(Bullet *bullet);
float DxBullet(Bullet *bullet);
float DyBullet(Bullet *bullet);
void moveBullet(Bullet *bullet);
void destroyBullet(Bullet *bullet);
SDL_Rect getBulletRect(Bullet *bullet);
void drawBullet(Bullet *bullet, SDL_Renderer *renderer);
bool checkCollisionBulletWall(Bullet *bullet, MAP *walls, int num_walls);
Bullet* createBullet(SDL_Renderer *renderer, float startX, float startY, int whoShot);

#endif