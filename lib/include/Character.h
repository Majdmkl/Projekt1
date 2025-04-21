#ifndef CHARACTER_H
#define CHARACTER_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Network.h"
#include "Bullet.h"
#include "Map.h"

#define MAX_HEALTH 3
#define MOVE_SPEED 5
#define CHARACTER_WIDTH 46
#define CHARACTER_HEIGHT 46


typedef struct character {
  float x;
  float y;
  float speed;
  int health;
  int frame;
  Uint32 lastFrameTime;
  SDL_Texture *walkRight;
  SDL_Texture *walkLeft;
  SDL_Texture *walkDown;
  SDL_Texture *walkUp;
  SDL_Texture *idleFront;
  enum { IDLE, WALKING_UP, WALKING_DOWN, WALKING_LEFT, WALKING_RIGHT } state;
} Character;

void turnUp(Character *pCharacter);
void turnLeft(Character *pCharacter);
void turnDown(Character *pCharacter);
void turnRight(Character *pCharacter);
int playerHealth(Character *character);
void decreaseHealth(Character *pCharacter);
bool isCharacterAlive(Character *pCharacter);
void destroyCharacter(Character *pCharacter);
void healthBar(Character *pCharacter, SDL_Renderer *renderer);
int howManyPlayersAlive(Character *players[], int num_players);
void renderCharacter(Character *pCharacter, SDL_Renderer *renderer);
bool checkCollision(Character *character, MAP *walls, int num_walls);
void updateCharacterAnimation(Character *pCharacter, Uint32 deltaTime);
Character *createCharacter(SDL_Renderer *renderer, int characterNumber);
bool checkCollisionCharacterBullet(Character *pCharacter, BulletData *bullet); // changed from Bullet to BulletData
void setBulletStartPosition(Character *pCharacter, float *startX, float *startY);
void moveCharacter(Character* character, float moveX, float moveY, MAP* walls, int wallCount);

#endif