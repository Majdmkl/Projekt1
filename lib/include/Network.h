#ifndef NETWORK_H
#define NETWORK_H

#define MIN_PLAYERS 2
#define MAX_ANIMALS 6
#define SERVER_PORT 2000

typedef enum { MENU, ONGOING  } GameState;
typedef struct { float x, y, dx, dy; } BulletData;
typedef enum { READY, UP, DOWN, LEFT, RIGHT, FIRE, BLOCKED, CONNECTING } ClientCommand;

typedef struct {
    int health, type;
    float x, y, speed_x, speed_y;
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
    int fire, whoShot, slotsTaken[MAX_ANIMALS];
    int numberOfBullets, numberOfPlayers;
    float bulletDx, bulletDy, bulletStartX, bulletStartY;
} ServerData;

#endif