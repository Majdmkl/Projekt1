#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>

#include "Network.h"
#include "Character.h"
#include "Bullet.h"
#include "Text.h"
#include "Map.h"

#define NR_OF_MENUTEXTURES 2

typedef enum { MAIN, SETTINGS, CONFIGURE, INGAME } MenuState;

typedef struct {
    char textureFiles[NR_OF_MENUTEXTURES][60];
    SDL_Texture *textures[NR_OF_MENUTEXTURES];
} MenuTextures;

typedef struct {
    TTF_Font *font;
    GameState state;
    ServerData server_data;
    UDPsocket socket;
    SDL_Rect menuRect;
    UDPpacket *packet;
    SDL_Window *window;
    MenuState menuState;
    SDL_Renderer *renderer;
    SDL_Rect backgroundRect;
    SDL_Texture *background;
    MenuTextures *menuTextures;
    Bullet *bullets[MAX_BULLETS];
    Text *waitingText, *joinedText;
    Character *players[MAX_ANIMALS];
    IPaddress serverAddress[MAX_ANIMALS];
    int numBullets, numPlayers, slotsTaken[MAX_ANIMALS], fire;
} Game;

void run(Game *game);
void close(Game *game);
int initiate(Game *game);
void renderCharacters(Game *game);
void sendGameData(Game *game, ClientData clientData);
void acceptClients(Game *game, ClientData clientData);
void executeCommand(Game *game, ClientData *clientData);
void addClient(IPaddress address, IPaddress clients[], int *numClients);

int main(int argc, char* argv[]) {
    Game game = {0};
    if (!initiate(&game)) return 1;
    run(&game);
    close(&game);
    return 0;
}

int initiate(Game *game) {
    srand((unsigned)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init Error: %s", SDL_GetError());
        return 0;
    }
    SDL_Log("SDL initialized.");

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "IMG_Init Error: %s", IMG_GetError());
        return 0;
    }
    SDL_Log("SDL_image initialized.");

    if (TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init Error: %s", TTF_GetError());
        return 0;
    }
    SDL_Log("SDL_ttf initialized.");

    game->font = TTF_OpenFont("lib/assets/fonts/PressStart2P.ttf", 60);
    if (!game->font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_OpenFont Error: %s", TTF_GetError());
        return 0;
    }
    SDL_Log("Font loaded.");

    game->window = SDL_CreateWindow("COZY TOWN Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!game->window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow Error: %s", SDL_GetError());
        return 0;
    }
    SDL_Log("Window created.");

    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!game->renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateRenderer Error: %s", SDL_GetError());
        return 0;
    }
    SDL_Log("Renderer created.");
    //! map here
    game->background = IMG_LoadTexture(game->renderer, "lib/assets/images/objects/buildings/Apartment.png");
    if (!game->background) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "IMG_LoadTexture Error: %s", IMG_GetError());
        return 0;
    }
    SDL_Log("Background texture loaded.");

    if (SDLNet_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDLNet_Init Error: %s", SDLNet_GetError());
        return 0;
    }
    SDL_Log("SDL_net initialized.");

    game->socket = SDLNet_UDP_Open(SERVER_PORT);
    if (!game->socket) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDLNet_UDP_Open Error: %s", SDLNet_GetError());
        return 0;
    }

    game->packet = SDLNet_AllocPacket(512);
    if (!game->packet) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDLNet_AllocPacket Error: %s", SDLNet_GetError());
        return 0;
    }
    SDL_Log("UDP socket opened on port %d.", SERVER_PORT);

    for (int i = 0; i < MAX_ANIMALS; i++) {
        game->players[i] = createCharacter(game->renderer, i);
        if (!game->players[i]) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createCharacter(%d) failed", i + 1);
            return 0;
        }
    }
    SDL_Log("Characters created.");

    game->waitingText = createText(game->renderer, 255, 255, 255, game->font, "Waiting for players...", SCREEN_WIDTH / 2 - 255, SCREEN_HEIGHT / 2 - 25);
    game->joinedText = createText(game->renderer, 255, 255, 255, game->font, "Player requirements met - Starting Game", SCREEN_WIDTH / 2 - 255, SCREEN_HEIGHT / 2 - 25);
    SDL_Log("Text objects created.");

    game->state = MAIN;
    game->numPlayers = 0;

    SDL_Log("Initialization completed successfully.");
    return 1;
}

void run(Game *game) {
    SDL_Event event;
    ClientData clientData = {0};
    ClientData lastClientData = {0};

    memset(game->slotsTaken, 0, sizeof(game->slotsTaken));
    game->numBullets = 0;

    int running = 1;

    while (running) {
        switch (game->state) {
            case INGAME:
                SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
                SDL_RenderClear(game->renderer);
                SDL_RenderCopy(game->renderer, game->background, NULL, NULL);
                renderCharacters(game);

                while (SDLNet_UDP_Recv(game->socket, game->packet) == 1) {
                    memcpy(&clientData, game->packet->data, sizeof(ClientData));
                    lastClientData = clientData;
                    executeCommand(game, &clientData);
                    memset(&clientData, 0, sizeof(clientData));
                }

                for (int i = 0; i < game->numBullets; i++) {
                    if (!game->bullets[i]) continue;
                    moveBullet(game->bullets[i]);
                    drawBullet(game->bullets[i], game->renderer);
                }
                for (int b = 0; b < game->numBullets; ) {
                    Bullet* bullet = game->bullets[b];
                    bool hit = false;
                    for (int p = 0; p < MAX_ANIMALS; p++) {
                        if (p != bullet->whoShot && checkCollisionCharacterBullet(game->players[p], bullet)) {
                            decreaseHealth(game->players[p]);
                            hit = true;
                            break;
                        }
                    }
                    if (hit || SDL_GetTicks() - getBulletBornTime(bullet) > BULLET_LIFETIME || checkCollisionBulletWall(bullet, walls, MAX_WALLS)) {
                        destroyBullet(bullet);
                        game->bullets[b] = game->bullets[--game->numBullets];
                    } else {
                        b++;
                    }
                }

                sendGameData(game, lastClientData);

                if (SDL_PollEvent(&event) && event.type == SDL_QUIT) running = 0;

                SDL_RenderPresent(game->renderer);
                SDL_Delay(16);
                break;

            case MAIN:
                SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
                SDL_RenderClear(game->renderer);

                if (SDLNet_UDP_Recv(game->socket, game->packet) == 1) {
                    memcpy(&clientData, game->packet->data, sizeof(ClientData));

                    if (clientData.command[0] == CONNECTING && clientData.playerNumber >= 0 && clientData.playerNumber < MAX_ANIMALS) {
                        int id = clientData.playerNumber;
                        if (!game->slotsTaken[id]) {
                            game->slotsTaken[id] = 1;
                            game->serverAddress[id] = game->packet->address;
                            game->numPlayers++;
                            sendGameData(game, clientData);
                            if (game->numPlayers >= MIN_PLAYERS) {
                                game->state = INGAME;
                                destroyText(game->waitingText);
                                drawText(game->joinedText);
                            }
                        }
                    }
                }

                drawText(game->waitingText);
                SDL_RenderPresent(game->renderer);

                if (SDL_PollEvent(&event) && event.type == SDL_QUIT) running = 0;
                break;
        }
    }
}

void acceptClients(Game *game, ClientData clientData) {
    while (game->numPlayers < MAX_ANIMALS) {
        if (SDLNet_UDP_Recv(game->socket, game->packet) == 1) {
            addClient(game->packet->address, game->serverAddress, &game->numPlayers);
            sendGameData(game, clientData);
        } else break;
    }
}

void renderCharacters(Game *game) {
    for (int i = 0; i < MAX_ANIMALS; i++) {
        if (game->slotsTaken[i]) {
            renderCharacter(game->players[i], game->renderer);
            healthBar(game->players[i], game->renderer);
        }
    }
}

void addClient(IPaddress address, IPaddress clients[], int *numClients) {
    for (int i = 0; i < *numClients; i++)
        if (address.host == clients[i].host && address.port == clients[i].port) return;
    clients[(*numClients)++] = address;
}

void executeCommand(Game *game, ClientData *clientData) {
    game->slotsTaken[clientData->playerNumber] = 1;
    Character *player = game->players[clientData->playerNumber];

    setPosition(player, clientData->animals.x, clientData->animals.y);
    if (clientData->command[1] == UP && clientData->command[6] != BLOCKED) turnUp(player);
    if (clientData->command[2] == DOWN && clientData->command[6] != BLOCKED) turnDown(player);
    if (clientData->command[3] == LEFT && clientData->command[6] != BLOCKED) turnLeft(player);
    if (clientData->command[4] == RIGHT && clientData->command[6] != BLOCKED) turnRight(player);

    if (clientData->command[5] == FIRE) {
        game->bullets[game->numBullets] = createBullet(
            game->renderer,
            clientData->bulletStartX, clientData->bulletStartY,
            clientData->bulletDx, clientData->bulletDy,
            clientData->playerNumber
        );

        if (game->bullets[game->numBullets]) {
            game->bullets[game->numBullets]->dx = clientData->bulletDx;
            game->bullets[game->numBullets]->dy = clientData->bulletDy;
            game->numBullets++;
            game->fire = 1;
        }
    }
}

void characterSendData(Character *character, Animal *animal) {
    animal->x = getX(character);
    animal->y = getY(character);
    animal->health = getPlayerHP(character);
    animal->type = getcharacterID(character);
    animal->speed_x = MOVE_SPEED;
    animal->speed_y = MOVE_SPEED;
}

void sendGameData(Game *game, ClientData clientData) {
    ServerData server_data = {0};
    server_data.gameState = game->state;
    server_data.numberOfPlayers = game->numPlayers;
    for (int i = 0; i < MAX_ANIMALS; i++) {
        if (getPlayerHP(game->players[i]) <= 0) game->slotsTaken[i] = 0;
        server_data.slotsTaken[i] = game->slotsTaken[i];
        characterSendData(game->players[i], &server_data.animals[i]);
    }

    server_data.numberOfBullets = game->numBullets;
    for (int i = 0; i < game->numBullets; i++) {
        Bullet *b = game->bullets[i];
        server_data.bullets[i].x = xBullet(b);
        server_data.bullets[i].y = yBullet(b);
        server_data.bullets[i].dx = DxBullet(b);
        server_data.bullets[i].dy = DyBullet(b);
        server_data.bullets[i].whoShot = b->whoShot;
    }

    memcpy(game->packet->data, &server_data, sizeof(ServerData));
    game->packet->len = sizeof(ServerData);

    for (int i = 0; i < game->numPlayers; i++) {
        game->packet->address = game->serverAddress[i];
        SDLNet_UDP_Send(game->socket, -1, game->packet);
    }
}

void close(Game *game) {
    for (int i = 0; i < MAX_ANIMALS; i++) if (game->players[i]) destroyCharacter(game->players[i]);
    for (int i = 0; i < game->numBullets; i++) if (game->bullets[i]) destroyBullet(game->bullets[i]);

    destroyText(game->waitingText);
    TTF_CloseFont(game->font);

    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    SDLNet_FreePacket(game->packet);
    SDLNet_UDP_Close(game->socket);

    TTF_Quit();
    SDLNet_Quit();
    SDL_Quit();
}