#ifndef CHARACTER_H
#define CHARACTER_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Network.h"
#include "Bullet.h"
#include "Map.h"

#define CHARACTER_HEIGHT 46
#define CHARACTER_WIDTH 46
#define MAX_HEALTH 4

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Network.h"
#include "Bullet.h"
#include "Map.h"


typedef struct character Character;

Character *createCharacter(SDL_Renderer *renderer, int characterNumber);
void decreaseHealth(Character *pCharacter);
int isCharacterAlive(Character *pCharacter);
void turnLeft(Character *pCharacter);
void turnRight(Character *pCharacter);
void turnUp(Character *pCharacter);
void turnDown(Character *pCharacter);
void updateCharacterAnimation(Character *pCharacter, Uint32 deltaTime);
void renderCharacter(Character *pCharacter, SDL_Renderer *renderer);
void destroyCharacter(Character *pCharacter);
void characterSendData(Character *pCharacter, MonkeyData *pMonkeyData);
void updateCharacterFromServer(Character *pCharacter, MonkeyData *pMonkeyData);
void healthBar(Character *pCharacter, SDL_Renderer *renderer);
bool checkCollisionCharacterBullet(Character *pCharacter, Bullet *bullet);
void setBulletStartPosition(Character *pCharacter, float *startX, float *startY);
bool checkCollision(Character *character, Wall *walls, int num_walls);
int howManyPlayersAlive(Character *players[], int num_players);
int playerHealth(Character *character);

#endif