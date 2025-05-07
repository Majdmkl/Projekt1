#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_net.h>

#include "Map.h"
#include "Text.h"
#include "Bullet.h"
#include "Character.h"
#include "Network.h"

void initSDL();
bool initNetwork();
void cleanupNetwork();
SDL_Window* createWindow();
bool connectToServer(const char* serverIP);
int selectCharacter(SDL_Renderer* renderer);
SDL_Renderer* createRenderer(SDL_Window* window);
void sendPlayerData(Character* player, int action);
void gameLoop(SDL_Renderer* renderer, Character* player);
void cleanup(SDL_Window* window, SDL_Renderer* renderer);
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath);
bool receiveServerData();
Character* createSelectedCharacter(SDL_Renderer* renderer, int selected);

UDPsocket clientSocket;
UDPpacket *sendPacket;
UDPpacket *receivePacket;
IPaddress serverIP;
int playerID = -1;
ServerData serverData;
bool connected = false;

int main(int argc, char* argv[]) {
    initSDL();

    if (!initNetwork()) {
        SDL_Log("Network initialization failed!");
        return 1;
    }

    SDL_Window* window = createWindow();
    SDL_Renderer* renderer = createRenderer(window);

    if (argc > 1) {
        if (!connectToServer(argv[1])) {
            SDL_Log("Failed to connect to server at %s", argv[1]);
            cleanup(window, renderer);
            cleanupNetwork();
            return 1;
        }
    } else {
        if (!connectToServer("127.0.0.1")) {
            SDL_Log("Failed to connect to local server");
            cleanup(window, renderer);
            cleanupNetwork();
            return 1;
        }
    }

    // Start menu - Start game
    int selected = selectCharacter(renderer);
    if (selected == -1) {
        cleanup(window, renderer);
        cleanupNetwork();
        return 1;
    }

    Character* player = createSelectedCharacter(renderer, selected);
    if (!player) {
        SDL_Log("Could not create character.");
        cleanup(window, renderer);
        cleanupNetwork();
        return 1;
    }

    ClientData initialData = {0};
    initialData.playerNumber = selected;
    initialData.command[0] = CONNECTING;
    initialData.animals.type = selected;

    memcpy(sendPacket->data, &initialData, sizeof(ClientData));
    sendPacket->len = sizeof(ClientData);
    SDLNet_UDP_Send(clientSocket, -1, sendPacket);

    Uint32 startTime = SDL_GetTicks();
    while (SDL_GetTicks() - startTime < 10000) {
        if (SDLNet_UDP_Recv(clientSocket, receivePacket)) {
            memcpy(&serverData, receivePacket->data, sizeof(ServerData));

            for (int i = 0; i < MAX_ANIMALS; i++) {
                if (serverData.slotsTaken[i] &&
                    serverData.animals[i].type == selected) {
                    playerID = i;
                    connected = true;
                    break;
                }
            }

            if (connected) break;
        } else SDLNet_UDP_Send(clientSocket, -1, sendPacket);  // resend connect request

        SDL_Delay(100);
    }

    if (!connected) {
        SDL_Log("Failed to connect with selected character.");
        destroyCharacter(player);
        cleanup(window, renderer);
        cleanupNetwork();
        return 1;
    }

    gameLoop(renderer, player);

    destroyCharacter(player);
    cleanup(window, renderer);
    cleanupNetwork();

    return 0;
}

void initSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    SDLNet_Init();
}

bool initNetwork() {
    if (SDLNet_Init() != 0) {
        SDL_Log("SDLNet_Init error: %s", SDLNet_GetError());
        return false;
    }

    clientSocket = SDLNet_UDP_Open(0); // 0 means any available port
    if (!clientSocket) {
        SDL_Log("SDLNet_UDP_Open error: %s", SDLNet_GetError());
        return false;
    }

    sendPacket = SDLNet_AllocPacket(512);
    receivePacket = SDLNet_AllocPacket(512);

    if (!sendPacket || !receivePacket) {
        SDL_Log("SDLNet_AllocPacket error: %s", SDLNet_GetError());
        return false;
    }

    return true;
}

void cleanupNetwork() {
    if (sendPacket) SDLNet_FreePacket(sendPacket);
    if (receivePacket) SDLNet_FreePacket(receivePacket);
    if (clientSocket) SDLNet_UDP_Close(clientSocket);
    SDLNet_Quit();
}

bool connectToServer(const char* serverIP_str) {
    if (SDLNet_ResolveHost(&serverIP, serverIP_str, SERVER_PORT) != 0) {
        SDL_Log("SDLNet_ResolveHost error: %s", SDLNet_GetError());
        return false;
    }

    sendPacket->address = serverIP;

    return true;
}


void sendPlayerData(Character* player, int action) {
    if (!connected) return;

    ClientData clientData = {0};
    clientData.playerNumber = playerID;
    clientData.animals.x = getX(player);
    clientData.animals.y = getY(player);

    switch (action) {
        case 1: clientData.command[1] = UP; break;
        case 2: clientData.command[2] = DOWN; break;
        case 3: clientData.command[3] = LEFT; break;
        case 4: clientData.command[4] = RIGHT; break;
        case 5:
            clientData.command[5] = FIRE;
            clientData.bulletStartX = getX(player) + CHARACTER_WIDTH / 2.0f;
            clientData.bulletStartY = getY(player) + CHARACTER_HEIGHT / 2.0f;

            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            clientData.bulletDx = mouseX - clientData.bulletStartX;
            clientData.bulletDy = mouseY - clientData.bulletStartY;
            break;
    }

    memcpy(sendPacket->data, &clientData, sizeof(ClientData));
    sendPacket->len = sizeof(ClientData);
    SDLNet_UDP_Send(clientSocket, -1, sendPacket);
}

bool receiveServerData() {
    if (SDLNet_UDP_Recv(clientSocket, receivePacket)) {
        memcpy(&serverData, receivePacket->data, sizeof(ServerData));
        return true;
    }
    return false;
}

SDL_Window* createWindow() {
    return SDL_CreateWindow("COZY DELIVERY", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

SDL_Renderer* createRenderer(SDL_Window* window) {
    return SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) { SDL_Log("Failed to load image: %s\n", IMG_GetError()); return NULL; }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

Character* createSelectedCharacter(SDL_Renderer* renderer, int selected) {
    Character* player = createCharacter(renderer, selected);
    if (!player) {
        SDL_Log("Failed to create character %d", selected);
        return NULL;
    }
    return player;
}

void gameLoop(SDL_Renderer* renderer, Character* player) {
    if (!player) {
        SDL_Log("Invalid player character");
        return;
    }

    MAP* gameMap = createMap(renderer);
    if (!gameMap) { SDL_Log("Failed to create map"); return; }

    Bullet* bullets[MAX_BULLETS];
    int bulletCount = 0;

    Character* otherPlayers[MAX_ANIMALS] = {NULL};
    bool playerActive[MAX_ANIMALS] = {false};

    SDL_Event event;
    bool running = true;
    Uint32 lastNetworkUpdate = 0;

    for (int i = 0; i < MAX_ANIMALS; i++) {
        if (i != playerID && serverData.slotsTaken[i]) {
            otherPlayers[i] = createCharacter(renderer, serverData.animals[i].type);
            if (otherPlayers[i]) {
                setPosition(otherPlayers[i], serverData.animals[i].x, serverData.animals[i].y);
                playerActive[i] = true;
            }
        }
    }

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                float startX = getX(player) + CHARACTER_WIDTH / 2.0f;
                float startY = getY(player) + CHARACTER_HEIGHT / 2.0f;

                if (bulletCount < MAX_BULLETS) {
                    bullets[bulletCount++] = createBullet(renderer, startX, startY, mouseX - startX, mouseY - startY, playerID);
                    sendPlayerData(player, 5); // 5 = FIRE command
                }
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        float moveX = 0, moveY = 0;
        int actionSent = 0;

        if (keys[SDL_SCANCODE_W]) {
            moveY -= MOVE_SPEED;
            turnUp(player);
            if (!actionSent) { sendPlayerData(player, 1); actionSent = 1; } // UP
        }
        if (keys[SDL_SCANCODE_S]) {
            moveY += MOVE_SPEED;
            turnDown(player);
            if (!actionSent) { sendPlayerData(player, 2); actionSent = 1; } // DOWN
        }
        if (keys[SDL_SCANCODE_A]) {
            moveX -= MOVE_SPEED;
            turnLeft(player);
            if (!actionSent) { sendPlayerData(player, 3); actionSent = 1; } // LEFT
        }
        if (keys[SDL_SCANCODE_D]) {
            moveX += MOVE_SPEED;
            turnRight(player);
            if (!actionSent) { sendPlayerData(player, 4); actionSent = 1; } // RIGHT
        }

        if (moveX != 0 && moveY != 0) {
            float diagSpeed = MOVE_SPEED / 1.4142f;
            moveX = (moveX > 0) ? diagSpeed : -diagSpeed;
            moveY = (moveY > 0) ? diagSpeed : -diagSpeed;
        }

        moveCharacter(player, moveX, moveY, walls, MAX_WALLS);
        updateCharacterAnimation(player, SDL_GetTicks());

        Uint32 now = SDL_GetTicks();
        if (now - lastNetworkUpdate > 50) { // 20 updates per second
            if (receiveServerData()) {
                for (int i = 0; i < MAX_ANIMALS; i++) {
                    if (i != playerID && serverData.slotsTaken[i]) {
                        if (!playerActive[i] || !otherPlayers[i]) {
                            otherPlayers[i] = createCharacter(renderer, serverData.animals[i].type);
                            if (otherPlayers[i]) playerActive[i] = true;
                        }

                        if (playerActive[i] && otherPlayers[i]) {
                            setPosition(otherPlayers[i], serverData.animals[i].x, serverData.animals[i].y);
                            int targetHP = serverData.animals[i].health;
                            while (getPlayerHP(otherPlayers[i]) > targetHP) decreaseHealth(otherPlayers[i]);
                        }
                    } else if (i != playerID && !serverData.slotsTaken[i] && playerActive[i]) {
                        if (otherPlayers[i]) destroyCharacter(otherPlayers[i]);
                        otherPlayers[i] = NULL;
                        playerActive[i] = false;
                    }
                }

                if (serverData.slotsTaken[playerID]) {
                    int srvHP = serverData.animals[playerID].health;
                    while (getPlayerHP(player) > srvHP) decreaseHealth(player);
                }

                if (serverData.fire && serverData.whoShot != playerID) {
                    if (bulletCount < MAX_BULLETS) {
                        bullets[bulletCount++] = createBullet(renderer,
                                                              serverData.bulletStartX,
                                                              serverData.bulletStartY,
                                                              serverData.bulletDx,
                                                              serverData.bulletDy,
                                                              serverData.whoShot);
                    }
                }
            }
            lastNetworkUpdate = now;
            sendPlayerData(player, 0);
        }

        for (int i = 0; i < bulletCount; ) {
            Bullet* b = bullets[i];
            bool bulletHit = false;

            if (checkCollisionCharacterBullet(player, b)) {
                decreaseHealth(player);
                bulletHit = true;
            }

            for (int j = 0; j < MAX_ANIMALS && !bulletHit; j++) {
                if (j != playerID && playerActive[j] && otherPlayers[j]) {
                    if (checkCollisionCharacterBullet(otherPlayers[j], b)) {
                        decreaseHealth(otherPlayers[j]);
                        bulletHit = true;
                    }
                }
            }

            if ((now - getBulletBornTime(b) > BULLET_LIFETIME) ||
                checkCollisionBulletWall(b, walls, MAX_WALLS) ||
                bulletHit) {
                destroyBullet(b);
                bullets[i] = bullets[--bulletCount];
            } else {
                moveBullet(b);
                ++i;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderMap(gameMap, renderer);
        renderCharacter(player, renderer);

        for (int i = 0; i < MAX_ANIMALS; i++) {
            if (i != playerID && playerActive[i] && otherPlayers[i]) {
                renderCharacter(otherPlayers[i], renderer);
                healthBar(otherPlayers[i], renderer);
            }
        }
        for (int i = 0; i < bulletCount; i++) drawBullet(bullets[i], renderer);

        healthBar(player, renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 fps
    }

    for (int i = 0; i < bulletCount; i++) destroyBullet(bullets[i]);
    for (int i = 0; i < MAX_ANIMALS; i++) if (i != playerID && playerActive[i]) destroyCharacter(otherPlayers[i]);

    destroyMap(gameMap);
}

int selectCharacter(SDL_Renderer* renderer) {
    SDL_Texture* menuTexture = loadTexture(renderer, "lib/assets/objects/meny.png");
    if (!menuTexture) {
        SDL_Log("Failed to load menu texture");
        return -1;
    }
    SDL_Texture* grassTexture = loadTexture(renderer, "lib/assets/objects/grass.png");
    if (!grassTexture) {
        SDL_DestroyTexture(menuTexture);
        SDL_Log("Failed to load grass texture");
        return -1;
    }

    SDL_Event event;
    int selected = -1;

    SDL_Rect menuRect = { (SCREEN_WIDTH - 384) / 2, (SCREEN_HEIGHT - 384) / 2, 384, 384 };

    int menuX = menuRect.x;
    int menuY = menuRect.y;

    SDL_Rect characters[MAX_ANIMALS] = {
        {menuX + 32,  menuY + 140,  64, 64},  // Panda
        {menuX + 185, menuY + 140,  64, 64},  // Giraffe
        {menuX + 288, menuY + 150,  64, 64},  // Fox
        {menuX + 43,  menuY + 280,  64, 64},  // Bear
        {menuX + 160, menuY + 270,  64, 64},  // Bunny
        {menuX + 280, menuY + 280,  64, 64}   // Lion
    };

    while (selected == -1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(menuTexture);
                SDL_DestroyTexture(grassTexture);
                return -1;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;
                for (int i = 0; i < 6; ++i) {
                    if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &characters[i])) {
                        selected = i;
                        break;
                    }
                }
            }
        }

        for (int y = 0; y < SCREEN_HEIGHT; y += TILE_SIZE) {
            for (int x = 0; x < SCREEN_WIDTH; x += TILE_SIZE) {
                SDL_Rect dst = { x, y, TILE_SIZE, TILE_SIZE };
                SDL_RenderCopy(renderer, grassTexture, NULL, &dst);
            }
        }

        SDL_RenderCopy(renderer, menuTexture, NULL, &menuRect);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(menuTexture);
    SDL_DestroyTexture(grassTexture);
    return selected;
}

void cleanup(SDL_Window* window, SDL_Renderer* renderer) {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    cleanupNetwork();
    SDL_Quit();
    IMG_Quit();
    SDLNet_Quit();
}