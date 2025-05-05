#include "Bullet.h"
#include <stdlib.h>
#include <math.h>

Bullet* createBullet(SDL_Renderer* renderer, float startX, float startY, float dirX, float dirY, int whoShot) {
    Bullet* bullet = (Bullet*)malloc(sizeof(Bullet));
    if (!bullet) return NULL;

    bullet->x = startX;
    bullet->y = startY;
    bullet->dx = 0;
    bullet->dy = 0;
    bullet->whoShot = whoShot;
    bullet->bornTime = SDL_GetTicks();

    return bullet;
}

void destroyBullet(Bullet* bullet) { free(bullet); }
float xBullet(Bullet* bullet) { return bullet->x; }
float yBullet(Bullet* bullet) { return bullet->y; }
float DxBullet(Bullet* bullet) { return bullet->dx; }
float DyBullet(Bullet* bullet) { return bullet->dy; }
Uint32 getBulletBornTime(Bullet* bullet) { return bullet->bornTime; }
void moveBullet(Bullet* bullet) { bullet->x += bullet->dx; bullet->y += bullet->dy; }

void drawBullet(Bullet* bullet, SDL_Renderer* renderer) {
    SDL_Rect rect = { (int)bullet->x, (int)bullet->y, 8, 8 };
    SDL_RenderCopy(renderer, bullet->texture, NULL, &rect);
}

SDL_Rect getBulletRect(Bullet* bullet) {
    SDL_Rect rect = { bullet->x, bullet->y, BULLET_WIDTH, BULLET_HEIGHT };
    return rect;
}

int checkCollisionBulletWall(Bullet* bullet, MAP* walls, int numWalls) {
    SDL_Rect bulletRect = getBulletRect(bullet);
    SDL_Rect wallRects[numWalls];

    convertWallsToRects(walls, wallRects, numWalls);
    for (int i = 0; i < numWalls; i++)
        if (SDL_HasIntersection(&bulletRect, &wallRects[i])) return 1;
    return 0;
}