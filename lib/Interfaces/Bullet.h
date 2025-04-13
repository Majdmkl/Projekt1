#ifndef BULLET_H
#define BULLET_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#include "../Interfaces/Game_logic.h"
#include "../Interfaces/Character.h"
#include "../Interfaces/Map.h"

#define BULLET_WIDTH 5
#define BULLET_HEIGHT 5
#define BULLET_SPEED 5
#define BULLETLIFETIME 60

typedef struct {
  int whoShot;
  float x, y, dx, dy;
  SDL_Texture *texture;
  Wall *walls[23];
} Bullet;

Bullet* createBullet(SDL_Renderer *renderer, float startX, float startY, int whoShot);
void destroyBullet(Bullet *bullet);
void moveBullet(Bullet *bullet);
SDL_Rect getBulletRect(Bullet *bullet);
void drawBullet(Bullet *bullet, SDL_Renderer *renderer);
float xBullet(Bullet *bullet);
float DxBullet(Bullet *bullet);
float yBullet(Bullet *bullet);
float DyBullet(Bullet *bullet);
bool checkCollisionBulletWall(Bullet *bullet, Wall *walls, int num_walls);

#endif