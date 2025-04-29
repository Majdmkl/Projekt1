#ifndef NETWORK_H
#define NETWORK_H

#define MAX_ANIMALS 6

typedef enum { READY, UP, DOWN, LEFT, RIGHT, FIRE, BLOCKED } ClientCommand;

typedef enum { MENU, ONGOING  } GameState;

typedef struct {
    int health;
    float x, y;
    float speed_x, speed_y;
} Animal;

typedef struct {
    Animal animals;
    int playerNumber;
    int slotsTaken[4];
    int numberOfBullets;
    ClientCommand command[7];
    float bulletDx, bulletDy;
    float bulletStartX, bulletStartY;
} ClientData;

typedef struct { float x, y, dx, dy; } BulletData;

typedef struct {
    int fire;
    int whoShot;
    GameState gState;
    int slotsTaken[4];
    int numberOfBullets;
    int numberOfPlayers;
    float bulletDx, bulletDy;
    Animal animals[MAX_ANIMALS];
    float bulletStartX, bulletStartY;
} ServerData;

#endif