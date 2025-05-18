#ifndef NETWORK_H
#define NETWORK_H

#include "Bullet.h"

#define MAX_PLAYERS 3
#define SERVER_PORT 2000

typedef enum { MENU, ONGOING  } GameState;
typedef struct { float x, y, dx, dy; int whoShot; } BulletData;
typedef enum { UP, DOWN, LEFT, RIGHT, FIRE, BLOCKED, CONNECTING, CONTINUE } ClientCommand;

typedef struct {
    int packages;
    int health, type;
    float x, y, speed_x, speed_y;
} Animal;

typedef struct {
    Animal animals;
    ClientCommand command[8];
    int playerNumber, slotsTaken[MAX_PLAYERS], numberOfBullets;
    float bulletDx, bulletDy, bulletStartX, bulletStartY;
} ClientData;

typedef struct {
    int readyCount;
    GameState gameState;
    Animal animals[MAX_PLAYERS];
    int slotsTaken[MAX_PLAYERS];
    BulletData bullets[MAX_BULLETS];
    int numberOfPlayers, numberOfBullets;
} ServerData;

#endif