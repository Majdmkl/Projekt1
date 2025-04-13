#ifndef NETDATA_H
#define NETDATA_H

#define MAX_ANIMALS 6
#define MAX_BULLETS 200

typedef enum { READY, UP, DOWN, LEFT, RIGHT, FIRE, BLOCKED } ClientCommand;

typedef enum { MENU, ONGOING } GameState;

typedef struct { float x, y, speed_x, speed_y; int health; } Animal;

typedef struct {
    ClientCommand command[7];
    Animal animals;
    int playerNumber;
    int slotsTaken[4];
    int numberOfBullets;
    float bulletStartX, bulletStartY;
    float bulletDx, bulletDy;
} ClientData;

typedef struct { float x, y, dx, dy; } BulletData;

typedef struct {
    Animal animals[MAX_ANIMALS];
    int fire;
    int slotsTaken[4];
    int numberOfBullets;
    int numberOfPlayers;
    int whoShot;
    GameState gState;
    float bulletStartX, bulletStartY;
    float bulletDx, bulletDy;
} ServerData;

#endif