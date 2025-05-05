#ifndef NETWORK_H
#define NETWORK_H

#define MAX_ANIMALS 6
#define SERVER_PORT 2000

typedef enum { READY, UP, DOWN, LEFT, RIGHT, FIRE, BLOCKED, CONNECTING } ClientCommand;

typedef enum { MENU, ONGOING  } GameState;

typedef struct {
    int health, type;
    float x, y, speed_x, speed_y;
} Animal;

typedef struct {
    Animal animals;
    ClientCommand command[7];
    int playerNumber, slotsTaken[6], numberOfBullets;
    float bulletDx, bulletDy, bulletStartX, bulletStartY;
} ClientData;

typedef struct { float x, y, dx, dy; } BulletData;

typedef struct {
    GameState gameState;
    Animal animals[MAX_ANIMALS];
    int fire, whoShot, slotsTaken[MAX_ANIMALS];
    int assignedID, numberOfBullets, numberOfPlayers;
    float bulletDx, bulletDy, bulletStartX, bulletStartY;;
} ServerData;

#endif