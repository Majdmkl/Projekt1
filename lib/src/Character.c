#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define FRAME_COUNT 3
#define FRAME_DELAY 100
#define DIRECTION_COUNT 4

#include "Character.h"
#include "Bullet.h"

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
    character->x = 500;
    character->y = 500;
    character->speed = MOVE_SPEED;
    character->health = MAX_HEALTH;
    character->frame = 0;
    character->lastFrameTime = SDL_GetTicks();
    character->state = IDLE;

    const char* characterType = NULL;
    switch (characterNumber) {
        case 0: characterType = "panda"; break;
        case 1: characterType = "giraffe"; break;
        case 2: characterType = "fox"; break;
        case 3: characterType = "bear"; break;
        case 4: characterType = "bunny"; break;
        case 5: characterType = "lion"; break;
        default:
            free(character);
            return NULL;
    }

    char path[100];
    sprintf(path, "lib/assets/animal/%s/%s_full_spritesheet.png", characterType, characterType);
    character->fullSheet = loadCharacterTexture(renderer, path);

    if (!character->fullSheet) {
        destroyCharacter(character);
        return NULL;
    }

    return character;
}

void turnUp(Character* character) {
    character->state = WALKING_UP;
}

void turnLeft(Character* character) {
    character->state = WALKING_LEFT;
}

void turnDown(Character* character) {
    character->state = WALKING_DOWN;
}

void turnRight(Character* character) {
    character->state = WALKING_RIGHT;
}

int playerHealth(Character* character) {
    return character->health;
}

void decreaseHealth(Character* character) {
    if (character && character->health > 0) character->health--;
}

bool isCharacterAlive(Character* character) {
    return character->health > 0;
}

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
        character->frame = 0; // stå still i mittenrutan (mitten av 3 frames)
    }
}

void renderCharacter(Character* character, SDL_Renderer* renderer) {
    int frameWidth = 64;
    int frameHeight = 64;

    int row;
    switch (character->state) {
        case WALKING_DOWN:  row = 0; break; // rad 0 = ner
        case WALKING_UP:    row = 1; break; // rad 1 = upp
        case WALKING_LEFT:  row = 2; break; // rad 2 = vänster
        case WALKING_RIGHT: row = 3; break; 
        default:            row = 0; break; // IDLE = front (ner)
    }

    SDL_Rect srcRect = {
        character->frame * frameWidth,
        row * frameHeight,
        frameWidth,
        frameHeight
    };

    SDL_Rect destRect = {
        character->x,
        character->y,
        CHARACTER_WIDTH,
        CHARACTER_HEIGHT
    };

    SDL_RenderCopy(renderer, character->fullSheet, &srcRect, &destRect);
}



void healthBar(Character* character, SDL_Renderer* renderer) {
    SDL_Rect healthRect = {
        character->x,
        character->y - 10,
        (CHARACTER_WIDTH * character->health) / MAX_HEALTH,
        5
    };

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthRect);
}

int howManyPlayersAlive(Character* players[], int num_players) {
    int aliveCount = 0;
    for (int i = 0; i < num_players; i++) if (isCharacterAlive(players[i])) aliveCount++;
    return aliveCount;
}

bool checkCollisionCharacterBullet(Character* character, BulletData* bullet) {
    return (character->x < bullet->x + 10 &&
            character->x + CHARACTER_WIDTH > bullet->x &&
            character->y < bullet->y + 10 &&
            character->y + CHARACTER_HEIGHT > bullet->y);
}

void setBulletStartPosition(Character* character, float* startX, float* startY) {
    *startX = character->x + CHARACTER_WIDTH / 2;
    *startY = character->y + CHARACTER_HEIGHT / 2;
}

void moveCharacter(Character* character, float moveX, float moveY, MAP* walls, int wallCount) {
    float prevX = character->x;
    float prevY = character->y;

    character->x += moveX;
    character->y += moveY;

    bool collision = false;
    for (int i = 0; i < wallCount; i++) {
        if (character->x + CHARACTER_WIDTH > walls[i].x_min &&
            character->x < walls[i].x_max &&
            character->y + CHARACTER_HEIGHT > walls[i].y_min &&
            character->y < walls[i].y_max) {
            collision = true;
            break;
        }
    }

    if (collision) {
        character->x = prevX;
        character->y = prevY;
    }
}