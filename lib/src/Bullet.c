#include <stdlib.h>
#include <math.h>

#include "Map.h"
#include "Bullet.h"
#include "Character.h"

float xBullet(Bullet *bullet) {
    return bullet->x;
}

float yBullet(Bullet *bullet) {
    return bullet->y;
}

float DxBullet(Bullet *bullet) {
    return bullet->dx;
}

float DyBullet(Bullet *bullet) {
    return bullet->dy;
}

void moveBullet(Bullet *bullet) {
    bullet->x += bullet->dx;
    bullet->y += bullet->dy;
}

void destroyBullet(Bullet *bullet) {
    if (bullet->texture) SDL_DestroyTexture(bullet->texture);
    free(bullet);
}

SDL_Rect getBulletRect(Bullet *bullet) {
    SDL_Rect rect = { (int)bullet->x, (int)bullet->y, BULLET_WIDTH, BULLET_HEIGHT };
    return rect;
}

void drawBullet(Bullet *bullet, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect rect = getBulletRect(bullet);
    SDL_RenderCopy(renderer, bullet->texture, NULL, &rect);
}

bool checkCollisionBulletWall(Bullet *bullet, MAP *walls, int num_walls) {
    SDL_Rect bulletRect = getBulletRect(bullet);
    SDL_Rect wallRects[num_walls];

    convertWallsToRects(walls, wallRects, num_walls);    //get wall rects

    for (int i = 0; i < num_walls; i++) if (SDL_HasIntersection(&bulletRect, &wallRects[i])) return true;

    return false;
}

Bullet* createBullet(SDL_Renderer *renderer, float startX, float startY, int whoShot) {
    Bullet* bullet = (Bullet*)malloc(sizeof(Bullet));
    if (!bullet) {
        SDL_Log("Failed to allocate memory for bullet");
        return NULL;
    }

    bullet->x = startX;
    bullet->y = startY;
    bullet->whoShot = whoShot;
    // Random direction for now
    bullet->dx = BULLET_SPEED * (rand() % 3 - 1);
    bullet->dy = BULLET_SPEED * (rand() % 3 - 1);

    if (bullet->dx == 0 && bullet->dy == 0) {
        bullet->dx = BULLET_SPEED;
    }

    SDL_Surface* surface = SDL_CreateRGBSurface(0, BULLET_WIDTH, BULLET_HEIGHT, 32, 0, 0, 0, 0);
    if (!surface) {
        SDL_Log("Failed to create bullet surface: %s", SDL_GetError());
        free(bullet);
        return NULL;
    }

    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));

    bullet->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!bullet->texture) {
        SDL_Log("Failed to create bullet texture: %s", SDL_GetError());
        free(bullet);
        return NULL;
    }

    return bullet;
}