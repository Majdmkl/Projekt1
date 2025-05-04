#ifndef NETWORK_H
#define NETWORK_H

#define MAX_ANIMALS 6

typedef enum { READY, UP, DOWN, LEFT, RIGHT, FIRE, BLOCKED } ClientCommand;

typedef enum { MENU, ONGOING  } GameState;

typedef struct {
    int health;
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
    int fire, whoShot, slotsTaken[6];;
    int numberOfBullets, numberOfPlayers;
    float bulletDx, bulletDy, bulletStartX, bulletStartY;;
} ServerData;

#endif