#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define FRAME_COUNT 3
#define FRAME_DELAY 100

#include "Character.h"

struct Character {
    float x, y, speed;
    int health, frame, characterID;
    Uint32 lastFrameTime;
    SDL_Texture *fullSheet;
    enum { IDLE, WALKING_UP, WALKING_DOWN, WALKING_LEFT, WALKING_RIGHT } state;
};

float getX(Character* character) { return character->x; }
float getY(Character* character) { return character->y; }
int getcharacterID(Character* character) { return character->characterID; }
float getSpeed(Character* character) { return character->speed; }

SDL_Texture* loadCharacterTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) {
        SDL_Log("Failed to load image: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

Character* createCharacter(SDL_Renderer* renderer, int characterNumber) {
    Character* character = (Character*)malloc(sizeof(Character));
    if (!character) {
        SDL_Log("Failed to allocate memory for character");
        return NULL;
    }

    character->speed = MOVE_SPEED;
    character->health = MAX_HEALTH;
    character->frame = 0;
    character->lastFrameTime = SDL_GetTicks();
    character->state = IDLE;

    const char* characterType = NULL;
    switch (characterNumber) {
        case 0: setPosition(character, 0, 0); characterType = "panda"; character->characterID = 0; break;
        case 1: setPosition(character, 0, SCREEN_HEIGHT - CHARACTER_HEIGHT); characterType = "giraffe"; character->characterID = 1; break;
        case 2: setPosition(character, SCREEN_WIDTH - CHARACTER_WIDTH, 0); characterType = "fox"; character->characterID = 2; break;
        case 3: setPosition(character, SCREEN_WIDTH - CHARACTER_WIDTH, SCREEN_HEIGHT - CHARACTER_HEIGHT); characterType = "bear"; character->characterID = 3; break;
        case 4: setPosition(character, (SCREEN_WIDTH - CHARACTER_WIDTH) / 2, (SCREEN_HEIGHT - CHARACTER_HEIGHT) / 2); characterType = "bunny"; character->characterID = 4; break;
        case 5: setPosition(character, SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 200); characterType = "lion"; character->characterID = 5; break;
        default: free(character); return NULL;
    }

    char path[256];
    sprintf(path, "lib/assets/images/character/animal/%s/%s_full_spritesheet.png", characterType, characterType);
    character->fullSheet = loadCharacterTexture(renderer, path);
    if (!character->fullSheet) {
        destroyCharacter(character);
        return NULL;
    }

    return character;
}

void turnUp(Character* character)    { character->state = WALKING_UP; }
void turnDown(Character* character)  { character->state = WALKING_DOWN; }
void turnLeft(Character* character)  { character->state = WALKING_LEFT; }
void turnRight(Character* character) { character->state = WALKING_RIGHT; }

int getPlayerHP(Character* character) { return character->health; }

void decreaseHealth(Character* character) {
    character->health -= BULLET_DAMAGE;
    if (character->health < 0) character->health = 0;
}

bool isCharacterAlive(Character* character) { return character->health > 0; }

void destroyCharacter(Character* character) {
    if (character) {
        if (character->fullSheet) SDL_DestroyTexture(character->fullSheet);
        free(character);
    }
}

void updateCharacterAnimation(Character* character, Uint32 deltaTime) {
    Uint32 currentTime = SDL_GetTicks();
    bool isMoving = (character->state == WALKING_UP ||
                     character->state == WALKING_DOWN ||
                     character->state == WALKING_LEFT ||
                     character->state == WALKING_RIGHT);

    if (isMoving && currentTime - character->lastFrameTime >= FRAME_DELAY) {
        character->frame = (character->frame + 1) % FRAME_COUNT;
        character->lastFrameTime = currentTime;
    } else if (!isMoving) {
        character->frame = 0;
    }
}

void setPosition(Character* character, float x, float y) {
    character->x = x;
    character->y = y;
}

void setDirection(Character* character) { character->state = IDLE; }

int getSpriteRowForState(int state) {
    switch (state) {
        case WALKING_DOWN:  return 0;
        case WALKING_UP:    return 1;
        case WALKING_LEFT:  return 2;
        case WALKING_RIGHT: return 3;
        case IDLE:
        default:            return 0;
    }
}

void renderCharacter(Character* character, SDL_Renderer* renderer) {
    if (!character || !renderer || !character->fullSheet) return;

    int spriteRow = getSpriteRowForState(character->state);

    SDL_Rect srcRect = {
        character->frame * CHARACTER_WIDTH,
        spriteRow * CHARACTER_HEIGHT,
        CHARACTER_WIDTH,
        CHARACTER_HEIGHT
    };

    SDL_Rect destRect = { (int)character->x, (int)character->y, CHARACTER_WIDTH, CHARACTER_HEIGHT };
    SDL_RenderCopy(renderer, character->fullSheet, &srcRect, &destRect);
}

void healthBar(Character* character, SDL_Renderer* renderer) {
    if (!character || !renderer) return;
    SDL_Rect healthRect = {
        (int)character->x,
        (int)character->y - 10,
        (CHARACTER_WIDTH * character->health) / MAX_HEALTH,
        5
    };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthRect);
}

int howManyPlayersAlive(Character* players[], int num_players) {
    int aliveCount = 0;
    for (int i = 0; i < num_players; i++) {
        if (players[i] && isCharacterAlive(players[i])) aliveCount++;
    }
    return aliveCount;
}

bool checkCollisionCharacterBullet(Character* character, Bullet* bullet) {
    if (!character || !bullet) return false;
    SDL_Rect characterRect = { (int)character->x, (int)character->y, CHARACTER_WIDTH, CHARACTER_HEIGHT };
    SDL_Rect bulletRect = getBulletRect(bullet);
    return SDL_HasIntersection(&characterRect, &bulletRect);
}

void setBulletStartPosition(Character* character, float* startX, float* startY) {
    if (!character || !startX || !startY) return;
    *startX = character->x + CHARACTER_WIDTH / 2;
    *startY = character->y + CHARACTER_HEIGHT / 2;
}

void moveCharacter(Character* character, float moveX, float moveY, MAP* walls, int wallCount) {
    if (!character || (!walls && wallCount > 0)) return;

    float prevX = character->x;
    float prevY = character->y;

    character->x += moveX;
    character->y += moveY;

    for (int i = 0; i < wallCount; i++) {
        if (character->x + CHARACTER_WIDTH > walls[i].x_min &&
            character->x < walls[i].x_max &&
            character->y + CHARACTER_HEIGHT > walls[i].y_min &&
            character->y < walls[i].y_max) {
            character->x = prevX;
            character->y = prevY;
            break;
        }
    }
}