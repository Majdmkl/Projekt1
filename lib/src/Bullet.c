#include "Bullet.h"
#include <stdlib.h>
#include <math.h>

Bullet* createBullet(SDL_Renderer* renderer, float x, float y, float dx, float dy, int whoShot) {
    Bullet* b = malloc(sizeof *b);
    if (!b) return NULL;

    b->x = x; b->y = y; b->dx = dx; b->dy = dy;
    b->whoShot = whoShot;
    b->bornTime = SDL_GetTicks();
    return b;
}

void destroyBullet(Bullet* bullet) { free(bullet); }
float xBullet(Bullet* b) { return b->x; }
float yBullet(Bullet* b) { return b->y; }
float DxBullet(Bullet* b) { return b->dx; }
float DyBullet(Bullet* b) { return b->dy; }

Uint32 getBulletBornTime(Bullet* b) { return b->bornTime; }
void moveBullet(Bullet* b) { b->x += b->dx; b->y += b->dy; }

void drawBullet(Bullet* bullet, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect rect = getBulletRect(bullet);
    SDL_RenderFillRect(renderer, &rect);
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