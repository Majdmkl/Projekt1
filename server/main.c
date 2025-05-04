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
    Character *players[MAX_PLAYERS];
    IPaddress serverAddress[MAX_PLAYERS];
    int numBullets, numPlayers, slotsTaken[6], fire;
} Game;

void run(Game *game);
void close(Game *game);
int initiate(Game *game);
void sendGameData(Game *game, ClientData clientData);
void acceptClients(Game *game, ClientData clientData);
void executeCommand(Game *game, ClientData clientData);
void addClient(IPaddress address, IPaddress clients[], int *numClients);

int main(int argc, char* argv) {
    Game game = {0};
    if (!initiate(&game)) return 1;
    run(&game);
    close(&game);
    return 0;
}

int initiate(Game *game) {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init Error: %s", SDL_GetError());
        return 0;
    }

    if (SDLNet_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDLNet_Init: %s", SDLNet_GetError());
        SDL_Quit();
        return 0;
    }

    if (TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init Error: %s", TTF_GetError());
        return 0;
    }

    game->font = TTF_OpenFont("assets/arial.ttf", 60);
    if (!game->font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_OpenFont: %s", TTF_GetError());
        return 0;
    }

    game->window = SDL_CreateWindow("COZY TOWN Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!game->window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow Error: %s", SDL_GetError());
        return 0;
    }

    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!game->renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateRenderer Error: %s", SDL_GetError());
        return 0;
    }

    game->background = IMG_LoadTexture(game->renderer, "../lib/resources/monkeyMap.png");
    if (!game->background) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "IMG_LoadTexture Error: %s", IMG_GetError());
        return 0;
    }

    game->socket = SDLNet_UDP_Open(2000);
    game->packet = SDLNet_AllocPacket(512);
    if (!game->socket || !game->packet) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "UDP Setup Error: %s", SDLNet_GetError());
        return 0;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        game->players[i] = createCharacter(game->renderer, i + 1);
        if (!game->players[i]) return 0;
    }

    game->waitingText = createText(game->renderer, 255, 255, 255, game->font, "Waiting for players...", 400, 400);
    game->joinedText = createText(game->renderer, 255, 255, 255, game->font, "Player requirements met - Starting Game", 400, 400);

    game->state = MENU;
    game->numPlayers = 0;
    return 1;
}

void acceptClients(Game *game, ClientData clientData) {
    while (game->numPlayers < MAX_PLAYERS) {
        if (SDLNet_UDP_Recv(game->socket, game->packet) == 1) {
            addClient(game->packet->address, game->serverAddress, &game->numPlayers);
            sendGameData(game, clientData);
        } else break;
    }
}

void renderCharacters(Game *game) {
    for (int i = 0; i < game->numPlayers; i++) renderCharacter(game->players[i], game->renderer);
}

void run(Game *game) {
    SDL_Event event;
    ClientData clientData = {0};

    memset(game->slotsTaken, 0, sizeof(game->slotsTaken));
    game->numBullets = 0;

    int running = 1;
    while (running) {
        switch (game->state) {
            case ONGOING:
                sendGameData(game, clientData);
                SDL_RenderCopy(game->renderer, game->background, NULL, NULL);
                renderCharacters(game);

                if (SDLNet_UDP_Recv(game->socket, game->packet) == 1) {
                    memcpy(&clientData, game->packet->data, sizeof(ClientData));
                    executeCommand(game, clientData);
                    sendGameData(game, clientData);
                    memset(&clientData, 0, sizeof(clientData));
                }

                for (int i = 0; i < game->numBullets; i++) {
                    if (!game->bullets[i]) continue;
                    drawBullet(game->bullets[i], game->renderer);
                    for (int j = 0; j < 3; j++) moveBullet(game->bullets[i]);
                }

                if (SDL_PollEvent(&event) && event.type == SDL_QUIT) running = 0;
                SDL_RenderPresent(game->renderer);
                break;

            case MENU:
                acceptClients(game, clientData);
                drawText(game->waitingText);
                SDL_RenderPresent(game->renderer);
                sendGameData(game, clientData);

                if (SDL_PollEvent(&event) && event.type == SDL_QUIT) running = 0;

                if (SDLNet_UDP_Recv(game->socket, game->packet) == 1) {
                    addClient(game->packet->address, game->serverAddress, &game->numPlayers);
                    sendGameData(game, clientData);
                    game->slotsTaken[game->numPlayers - 1] = 1;

                    if (game->numPlayers == MAX_MONKEYS) {
                        game->state = ONGOING;
                        destroyText(game->waitingText);
                    }
                }
                break;
        }
    }
}

void addClient(IPaddress address, IPaddress clients[], int *numClients) {
    for (int i = 0; i < *numClients; i++) if (address.host == clients[i].host && address.port == clients[i].port) return;
    clients[(*numClients)++] = address;
}

void executeCommand(Game *game, ClientData clientData) {
    Character *player = game->players[clientData.playerNumber];
    if (clientData.command[1] == UP && clientData.command[6] != BLOCKED) turnUp(player);
    if (clientData.command[2] == DOWN && clientData.command[6] != BLOCKED) turnDown(player);
    if (clientData.command[3] == LEFT && clientData.command[6] != BLOCKED) turnLeft(player);
    if (clientData.command[4] == RIGHT && clientData.command[6] != BLOCKED) turnRight(player);

    if (clientData.command[5] == FIRE) {
        game->bullets[game->numBullets] = createBullet(game->renderer, clientData.bulletStartX, clientData.bulletStartY, clientData.playerNumber);
        if (game->bullets[game->numBullets]) {
            game->bullets[game->numBullets]->dx = clientData.bulletDx;
            game->bullets[game->numBullets]->dy = clientData.bulletDy;
            game->numBullets++;
            game->fire = 1;
        }
    }
}

void sendGameData(Game *game, ClientData clientData) {
    ServerData server_data = {0};
    server_data.gState = game->state;
    server_data.whoShot = clientData.playerNumber;
    server_data.fire = game->fire;
    game->fire = 0;

    for (int i = 0; i < MAX_MONKEYS; i++) {
        server_data.slotsTaken[i] = game->slotsTaken[i];
        characterSendData(game->players[i], &server_data.monkeys[i]);
    }

    if (game->numBullets > 0) {
        Bullet *lastBullet = game->bullets[game->numBullets - 1];
        server_data.bulletDx = DxBullet(lastBullet);
        server_data.bulletDy = DyBullet(lastBullet);
        server_data.bulletStartX = xBullet(lastBullet);
        server_data.bulletStartY = yBullet(lastBullet);
    }

    server_data.numberOfPlayers = game->numPlayers;
    server_data.numberOfBullets = game->numBullets;

    memcpy(game->packet->data, &server_data, sizeof(ServerData));
    game->packet->len = sizeof(ServerData);

    for (int i = 0; i < game->numPlayers; i++) {
        game->packet->address = game->serverAddress[i];
        SDLNet_UDP_Send(game->socket, -1, game->packet);
    }
}

void close(Game *game) {
    for (int i = 0; i < MAX_PLAYERS; i++) if (game->players[i]) destroyCharacter(game->players[i]);
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