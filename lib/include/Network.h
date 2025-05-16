#ifndef NETWORK_H
#define NETWORK_H

#include "Bullet.h"

#define MIN_PLAYERS 2
#define MAX_ANIMALS 2
#define SERVER_PORT 2000

typedef enum { MENU, ONGOING  } GameState;
typedef struct { float x, y, dx, dy; int whoShot; } BulletData;
typedef enum { READY, UP, DOWN, LEFT, RIGHT, FIRE, BLOCKED, CONNECTING } ClientCommand;

typedef struct {
    int health, type;
    float x, y, speed_x, speed_y;
    int packages;
} Animal;

typedef struct {
    Animal animals;
    ClientCommand command[7];
    int playerNumber, slotsTaken[MAX_ANIMALS], numberOfBullets;
    float bulletDx, bulletDy, bulletStartX, bulletStartY;
} ClientData;

typedef struct {
    GameState gameState;
    Animal animals[MAX_ANIMALS];
    int slotsTaken[MAX_ANIMALS];
    BulletData bullets[MAX_BULLETS];
    int numberOfPlayers, numberOfBullets;
} ServerData;

#endif