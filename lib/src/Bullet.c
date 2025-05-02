#include "Bullet.h"
#include <stdlib.h>
#include <math.h>

struct Bullet {
    int whoShot;
    float x, y;
    float dx, dy;
    Uint32 bornTime;
    SDL_Texture* texture;
};

Bullet* createBullet(SDL_Renderer* renderer, float startX, float startY, float dirX, float dirY, int whoShot) {
    Bullet* bullet = (Bullet*)malloc(sizeof(Bullet));
    if (!bullet) return NULL;

    float length = sqrtf(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        bullet->dx = (dirX / length) * BULLET_SPEED;
        bullet->dy = (dirY / length) * BULLET_SPEED;
    } else {
        bullet->dx = BULLET_SPEED;
        bullet->dy = 0;
    }

    bullet->x = startX;
    bullet->y = startY;
    bullet->whoShot = whoShot;
    bullet->bornTime = SDL_GetTicks();

    bullet->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 8, 8);
    if (bullet->texture) {
        SDL_SetRenderTarget(renderer, bullet->texture);
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, NULL);
    }

    return bullet;
}

void moveBullet(Bullet* bullet) { bullet->x += bullet->dx; bullet->y += bullet->dy; }

void drawBullet(Bullet* bullet, SDL_Renderer* renderer) {
    SDL_Rect rect = { (int)bullet->x, (int)bullet->y, 8, 8 };
    SDL_RenderCopy(renderer, bullet->texture, NULL, &rect);
}

void destroyBullet(Bullet* bullet) {
    if (bullet) {
        if (bullet->texture) SDL_DestroyTexture(bullet->texture);
        free(bullet);
    }
}

int checkCollisionBulletWall(Bullet* bullet, MAP* walls, int numWalls) {
    for (int i = 0; i < numWalls; i++)
        if (bullet->x >= walls[i].x_min && bullet->x <= walls[i].x_max && bullet->y >= walls[i].y_min && bullet->y <= walls[i].y_max) return 1;
    return 0;
}

Uint32 getBulletBornTime(Bullet* bullet) { return bullet->bornTime; }