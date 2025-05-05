#ifndef CHARACTER_H
#define CHARACTER_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Map.h"
#include "Bullet.h"
#include "Network.h"

#define MAX_HEALTH 3
#define MOVE_SPEED 5
#define CHARACTER_WIDTH 54
#define CHARACTER_HEIGHT 54

typedef struct Character Character;

int getType(Character* character);
float getX(Character *pCharacter);
float getY(Character *pCharacter);
void turnUp(Character *pCharacter);
float getSpeed(Character* character);
void turnLeft(Character *pCharacter);
void turnDown(Character *pCharacter);
void turnRight(Character *pCharacter);
int playerHealth(Character *character);
void setDirection(Character *pCharacter);
void decreaseHealth(Character *pCharacter);
bool isCharacterAlive(Character *pCharacter);
void destroyCharacter(Character *pCharacter);
void setPosition(Character *pCharacter, float x, float y);
void healthBar(Character *pCharacter, SDL_Renderer *renderer);
int howManyPlayersAlive(Character *players[], int num_players);
void renderCharacter(Character *pCharacter, SDL_Renderer *renderer);
bool checkCollision(Character *character, MAP *walls, int num_walls);
void updateCharacterAnimation(Character *pCharacter, Uint32 deltaTime);
Character *createCharacter(SDL_Renderer *renderer, int characterNumber);
bool checkCollisionCharacterBullet(Character *character, BulletData *bullet); // changed from Bullet to BulletData
void setBulletStartPosition(Character *pCharacter, float *startX, float *startY);
void moveCharacter(Character* character, float moveX, float moveY, MAP* walls, int wallCount);

#endif