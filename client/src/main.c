#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_ttf.h>

#include "Map.h"
#include "Bullet.h"
#include "Character.h"
#include "Network.h"

void initSDL();
bool initNetwork();
void cleanupNetwork();
SDL_Window* createWindow();
int mainMenu(SDL_Renderer* renderer); //!
void waitingRoom(SDL_Renderer* renderer); //!
bool connectToServer(const char* serverIP);
int selectCharacter(SDL_Renderer* renderer);
char* connectionScreen(SDL_Renderer* renderer); //!
SDL_Renderer* createRenderer(SDL_Window* window);
void sendPlayerData(Character* player, int action);
void gameLoop(SDL_Renderer* renderer, Character* player);
void cleanup(SDL_Window* window, SDL_Renderer* renderer);
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath);
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

    if (!initNetwork()) {  SDL_Log("Network initialization failed!"); return 1; }

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

    int menuSelection = mainMenu(renderer);
    if (menuSelection == 1) {
        char* ip = connectionScreen(renderer);
        if(ip) {
            int selected = selectCharacter(renderer);
            if(selected == -1) {
                cleanup(window, renderer);
                cleanupNetwork();
                return 1;
            }

            waitingRoom(renderer);

            Character * player = createSelectedCharacter(renderer, selected);
            if (!player) {
                SDL_Log("Failed to create character");
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

        cleanup(window, renderer);
        cleanupNetwork();
        return 0;
    }

    if (menuSelection == 2) {
        cleanup(window, renderer);
        cleanupNetwork();
        return 0;
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

    // 👉 Starta spelet
    gameLoop(renderer, player);

    destroyCharacter(player);
    cleanup(window, renderer);
    cleanupNetwork();

    return 0;
}

void initSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    SDLNet_Init();
}

bool initNetwork() {
    if (SDLNet_Init() != 0) { SDL_Log("SDLNet_Init error: %s", SDLNet_GetError()); return false; }

    clientSocket = SDLNet_UDP_Open(0);
    if (!clientSocket) { SDL_Log("SDLNet_UDP_Open error: %s", SDLNet_GetError()); return false; }

    sendPacket = SDLNet_AllocPacket(512);
    receivePacket = SDLNet_AllocPacket(512);
    if (!sendPacket || !receivePacket) { SDL_Log("SDLNet_AllocPacket error: %s", SDLNet_GetError()); return false; }

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
            float ddx = mouseX - clientData.bulletStartX;
            float ddy = mouseY - clientData.bulletStartY;
            float mag = sqrtf(ddx * ddx + ddy * ddy);
            if (mag > 0.0f) {
                clientData.bulletDx = ddx / mag * MOVE_SPEED;
                clientData.bulletDy = ddy / mag * MOVE_SPEED;
            } else {
                clientData.bulletDx = 0;
                clientData.bulletDy = 0;
            }
            break;
    }

    memcpy(sendPacket->data, &clientData, sizeof(ClientData));
    sendPacket->len = sizeof(ClientData);
    SDLNet_UDP_Send(clientSocket, -1, sendPacket);
}

SDL_Window* createWindow() {return SDL_CreateWindow("COZY DELIVERY", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0); }

SDL_Renderer* createRenderer(SDL_Window* window) { return SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); }

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) { SDL_Log("Failed to load image: %s\n", IMG_GetError()); return NULL; }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

Character* createSelectedCharacter(SDL_Renderer* renderer, int selected) {
    Character* player = createCharacter(renderer, selected);
    if (!player) { SDL_Log("Failed to create character %d", selected); return NULL; }
    return player;
}

void gameLoop(SDL_Renderer* renderer, Character* player) {
    if (!player) { SDL_Log("Invalid player character"); return; }

    MAP* gameMap = createMap(renderer);
    if (!gameMap) { SDL_Log("Failed to create map"); return; }

    Bullet* bullets[MAX_BULLETS];
    int bulletCount = 0;

    Character* otherPlayers[MAX_ANIMALS] = {NULL};
    bool playerActive[MAX_ANIMALS] = {false};

    SDL_Event event;
    bool running = true;
    bool spectating = false;
    float deathX = 0, deathY = 0;
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
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && !spectating) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                float startX = getX(player) + CHARACTER_WIDTH / 2.0f;
                float startY = getY(player) + CHARACTER_HEIGHT / 2.0f;

                sendPlayerData(player, 5);
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        float moveX = 0, moveY = 0;
        bool actionSent = 0;

        if (!spectating) {
            if (keys[SDL_SCANCODE_W]) {
                moveY -= MOVE_SPEED;
                turnUp(player);
                if (!actionSent) { sendPlayerData(player, 1); actionSent = true; } // UP
            }
            if (keys[SDL_SCANCODE_S]) {
                moveY += MOVE_SPEED;
                turnDown(player);
                if (!actionSent) { sendPlayerData(player, 2); actionSent = true; } // DOWN
            }
            if (keys[SDL_SCANCODE_A]) {
                moveX -= MOVE_SPEED;
                turnLeft(player);
                if (!actionSent) { sendPlayerData(player, 3); actionSent = true; } // LEFT
            }
            if (keys[SDL_SCANCODE_D]) {
                moveX += MOVE_SPEED;
                turnRight(player);
                if (!actionSent) { sendPlayerData(player, 4); actionSent = true; } // RIGHT
            }
        }

        if (moveX != 0 && moveY != 0) {
            float diagSpeed = MOVE_SPEED / 1.4142f;
            moveX = (moveX > 0) ? diagSpeed : -diagSpeed;
            moveY = (moveY > 0) ? diagSpeed : -diagSpeed;
        }

        float prevPlayerX = getX(player);
        float prevPlayerY = getY(player);
        moveCharacter(player, moveX, moveY, walls, MAX_WALLS);

        for (int i = 0; i < MAX_ANIMALS; i++) {
            if (i != playerID && playerActive[i] && otherPlayers[i]) {
                SDL_Rect pRect = { getX(player), getY(player), CHARACTER_WIDTH, CHARACTER_HEIGHT };
                SDL_Rect oRect = { getX(otherPlayers[i]), getY(otherPlayers[i]), CHARACTER_WIDTH, CHARACTER_HEIGHT };
                if (SDL_HasIntersection(&pRect, &oRect)) { setPosition(player, prevPlayerX, prevPlayerY); break; }
            }
        }
        updateCharacterAnimation(player, SDL_GetTicks());

        Uint32 now = SDL_GetTicks();
        if (now - lastNetworkUpdate > 16) { // ~60fps network tick
            bool gotData = false;
            while (SDLNet_UDP_Recv(clientSocket, receivePacket)) {
                memcpy(&serverData, receivePacket->data, sizeof(ServerData));
                gotData = true;
            }
            if (gotData) {
                for (int i = 0; i < MAX_ANIMALS; i++) {
                    if (i != playerID && serverData.slotsTaken[i]) {
                        if (!playerActive[i] || !otherPlayers[i]) {
                            otherPlayers[i] = createCharacter(renderer, serverData.animals[i].type);
                            playerActive[i] = otherPlayers[i] != NULL;
                        }
                        if (playerActive[i]) {
                            float oldX = getX(otherPlayers[i]);
                            float oldY = getY(otherPlayers[i]);
                            float newX = serverData.animals[i].x;
                            float newY = serverData.animals[i].y;
                            float dx = newX - oldX, dy = newY - oldY;
                            if (fabsf(dx) < 0.1f && fabsf(dy) < 0.1f) setDirection(otherPlayers[i]);
                            else if (fabsf(dx) > fabsf(dy))(dx > 0 ? turnRight : turnLeft)(otherPlayers[i]);
                            else (dy > 0 ? turnDown : turnUp)(otherPlayers[i]);

                            setPosition(otherPlayers[i], newX, newY);
                            updateCharacterAnimation(otherPlayers[i], SDL_GetTicks());
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

                for (int i = 0; i < bulletCount; ++i) destroyBullet(bullets[i]);
                bulletCount = 0;
                for (int i = 0; i < serverData.numberOfBullets && i < MAX_BULLETS; ++i) {
                    BulletData *bd = &serverData.bullets[i];
                    bullets[bulletCount++] = createBullet(renderer, bd->x, bd->y, bd->dx, bd->dy, bd->whoShot);
                }
            }
            if (!spectating && (!serverData.slotsTaken[playerID] || getPlayerHP(player) <= 0)) {
                spectating = true;
                deathX = getX(player);
                deathY = getY(player);
                destroyCharacter(player);
                player = NULL;
            }

            if (!spectating) sendPlayerData(player, 0);

            lastNetworkUpdate = now;
        }
        for (int i = 0; i < MAX_ANIMALS; ++i)
            if (i != playerID && playerActive[i] && otherPlayers[i]) updateCharacterAnimation(otherPlayers[i], now);

        for (int i = 0; i < bulletCount; ) {
            Bullet* b = bullets[i];
            if ((now - getBulletBornTime(b) > BULLET_LIFETIME) ||
                checkCollisionBulletWall(b, walls, MAX_WALLS)) {
                destroyBullet(b);
                bullets[i] = bullets[--bulletCount];
            } else { moveBullet(b); ++i; }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderMap(gameMap, renderer);
        if (player && getPlayerHP(player) > 0) {
            renderCharacter(player, renderer);
            healthBar(player, renderer);
        } else if (spectating) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect deathRect = { (int)deathX, (int)deathY, CHARACTER_WIDTH, CHARACTER_HEIGHT };
            SDL_RenderFillRect(renderer, &deathRect);
        }

        for (int i = 0; i < MAX_ANIMALS; i++) {
            if (i != playerID && playerActive[i] && otherPlayers[i]) {
                if (getPlayerHP(otherPlayers[i]) > 0) renderCharacter(otherPlayers[i], renderer);
                healthBar(otherPlayers[i], renderer);
            }
        }
        for (int i = 0; i < bulletCount; i++) drawBullet(bullets[i], renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 fps
    }

    for (int i = 0; i < bulletCount; i++) destroyBullet(bullets[i]);
    for (int i = 0; i < MAX_ANIMALS; i++) if (i != playerID && playerActive[i]) destroyCharacter(otherPlayers[i]);

    destroyMap(gameMap);
}
char* connectionScreen(SDL_Renderer* renderer) {
    static char ip[64] = "";
    bool typingActive = false;

    SDL_Texture* menuTexture = loadTexture(renderer, "lib/assets/images/ui/ip.png");
    SDL_Texture* grassTexture = loadTexture(renderer, "lib/assets/images/objects/nature/grass.png");

    TTF_Font* font = TTF_OpenFont("lib/assets/fonts/PressStart2P-Regular.ttf", 16);
    if (!font) { SDL_Log("Failed to load font: %s", TTF_GetError()); return NULL; }

    SDL_Rect menuRect = { (SCREEN_WIDTH - 800) / 2, (SCREEN_HEIGHT - 900) / 2, 800, 900 };
    SDL_Rect inputBox = { menuRect.x + 240, menuRect.y + 450, 340, 70 };
    SDL_Rect closeBtn = { inputBox.x + inputBox.w - 45, inputBox.y + 15, 40, 40 };

    SDL_Event event;
    bool running = true;

     while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return NULL;

            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &inputBox)) { typingActive = true; SDL_StartTextInput(); }
                else { typingActive = false; SDL_StopTextInput(); }

                if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &closeBtn)) {
                    ip[0] = '\0';
                    typingActive = false;
                    SDL_StopTextInput();
                }
            }
            else if (typingActive && event.type == SDL_TEXTINPUT) strncat(ip, event.text.text, sizeof(ip) - strlen(ip) - 1);
            else if (typingActive && event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(ip) > 0) ip[strlen(ip) - 1] = '\0';
                if (event.key.keysym.sym == SDLK_RETURN) {
                    SDL_StopTextInput();
                    SDL_DestroyTexture(menuTexture);
                    SDL_DestroyTexture(grassTexture);
                    TTF_CloseFont(font);
                    return ip;
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    SDL_StopTextInput();
                    SDL_DestroyTexture(menuTexture);
                    SDL_DestroyTexture(grassTexture);
                    TTF_CloseFont(font);
                    return NULL;
                }
            }
        }

        // === RENDERING ===
        // 1. Bakgrundsgräs
        for (int y = 0; y < SCREEN_HEIGHT; y += 64) {
            for (int x = 0; x < SCREEN_WIDTH; x += 64) {
                SDL_Rect dst = { x, y, 64, 64 };
                SDL_RenderCopy(renderer, grassTexture, NULL, &dst);
            }
        }

        // 2. Mörk overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 80);
        SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);

        // 3. Menybild
        SDL_RenderCopy(renderer, menuTexture, NULL, &menuRect);

        // 4. Tjock outline runt meny
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int i = 0; i < 4; i++) {
            SDL_Rect outline = { menuRect.x - i, menuRect.y - i, menuRect.w + 2 * i, menuRect.h + 2 * i };
            SDL_RenderDrawRect(renderer, &outline);
        }

        // 5. Tjock vit outline runt inputBox
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // for (int i = 0; i < 2; i++) {
        //     SDL_Rect inputOutline = {
        //         inputBox.x - i,
        //         inputBox.y - i,
        //         inputBox.w + 2 * i,
        //         inputBox.h + 2 * i
        //     };
        //     SDL_RenderDrawRect(renderer, &inputOutline);
        // }

        // 6. Röd outline runt closeBtn
        // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        // for (int i = 0; i < 2; i++) {
        //     SDL_Rect closeOutline = {
        //         closeBtn.x - i,
        //         closeBtn.y - i,
        //         closeBtn.w + 2 * i,
        //         closeBtn.h + 2 * i
        //     };
        //     SDL_RenderDrawRect(renderer, &closeOutline);
        // }

        // 7. Visa IP-texten
        if (strlen(ip) > 0) {
            SDL_Color textColor = {20, 120, 20};
            SDL_Surface* textSurface = TTF_RenderText_Blended(font, ip, textColor);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

            SDL_Rect textRect = { inputBox.x + 10, inputBox.y + (inputBox.h - textSurface->h) / 2, textSurface->w, textSurface->h };

            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    SDL_DestroyTexture(menuTexture);
    SDL_DestroyTexture(grassTexture);
    TTF_CloseFont(font);
    return NULL;
}

void waitingRoom(SDL_Renderer* renderer) {
    SDL_Texture* bgTexture = loadTexture(renderer, "lib/assets/images/ui/waitingRoom.png");
    SDL_Texture* grassTexture = loadTexture(renderer, "lib/assets/images/objects/nature/grass.png");

    SDL_Event event;

    SDL_Rect menuRect = {(SCREEN_WIDTH - 700) / 2, (SCREEN_HEIGHT - 900) / 2, 690, 910};
    SDL_Rect continueBtn = { menuRect.x + 170, menuRect.y + 765, 390, 85 };

    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int x = event.button.x;
                int y = event.button.y;
                if (SDL_PointInRect(&(SDL_Point){x, y}, &continueBtn)) {
                    SDL_DestroyTexture(bgTexture);
                    SDL_DestroyTexture(grassTexture);
                    return;
                }
            }
        }

        // Bakgrundsgräs
        for (int y = 0; y < SCREEN_HEIGHT; y += 64) {
            for (int x = 0; x < SCREEN_WIDTH; x += 64) {
                SDL_Rect dst = { x, y, 64, 64 };
                SDL_RenderCopy(renderer, grassTexture, NULL, &dst);
            }
        }

        // Mörk overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 80);
        SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);

        // Visa waitingRoom-bilden i mitten med rätt storlek
        SDL_RenderCopy(renderer, bgTexture, NULL, &menuRect);

        // Svart tjock outline
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int i = 0; i < 4; i++) {
            SDL_Rect outline = { menuRect.x - i, menuRect.y - i, menuRect.w + 2 * i, menuRect.h + 2 * i };
            SDL_RenderDrawRect(renderer, &outline);
        }

        // Vit ruta runt continue-knapp (för debug)
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // SDL_RenderDrawRect(renderer, &continueBtn);

        SDL_RenderPresent(renderer);
    }
}

int mainMenu(SDL_Renderer* renderer) {
    SDL_Texture* menuTexture = loadTexture(renderer, "lib/assets/images/ui/startMenyn.png");
    SDL_Texture* grassTexture = loadTexture(renderer, "lib/assets/images/objects/nature/grass.png");

    SDL_Event event;
    int selection = -1;

    SDL_Rect menuRect = { (SCREEN_WIDTH - 700) / 2, (SCREEN_HEIGHT - 900) / 2, 700, 900 };

    int menuX = menuRect.x;
    int menuY = menuRect.y;

    SDL_Rect startBtn = { menuX + 170, menuY + 280, 360, 85 };
    SDL_Rect quitBtn  = { menuX + 170, menuY + 470, 360, 85 };
    SDL_Rect connectBtn = { menuX + 170, menuY + 375, 360, 85 };

    while (selection == -1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return 2;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &startBtn)) selection = 0;
                else if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &connectBtn)) selection = 1;
                else if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &quitBtn)) selection = 2;
            }
        }

        for (int y = 0; y < SCREEN_HEIGHT; y += 64) {
            for (int x = 0; x < SCREEN_WIDTH; x += 64) {
                SDL_Rect dst = { x, y, 64, 64 };
                SDL_RenderCopy(renderer, grassTexture, NULL, &dst);
            }
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 80);
        SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);

        SDL_RenderCopy(renderer, menuTexture, NULL, &menuRect);

        // Välj färg för outline
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // svart

        // Tjocklek i pixlar
        int thickness = 4;

        for (int i = 0; i < thickness; i++) {
            SDL_Rect outline = { menuRect.x - i, menuRect.y - i, menuRect.w + 2 * i, menuRect.h + 2 * i };
            SDL_RenderDrawRect(renderer, &outline);
        }

        //Rita outlines
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Vit färg
        // SDL_RenderDrawRect(renderer, &startBtn);
        // SDL_RenderDrawRect(renderer, &quitBtn);
        // SDL_RenderDrawRect(renderer, &connectBtn);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(menuTexture);
    SDL_DestroyTexture(grassTexture);
    return selection;
}


int selectCharacter(SDL_Renderer* renderer) {
    SDL_Texture* menuTexture = loadTexture(renderer, "lib/assets/images/ui/selChar.png");
    SDL_Texture* grassTexture = loadTexture(renderer, "lib/assets/images/objects/nature/grass.png");

    SDL_Event event;
    int selected = -1;

    SDL_Rect menuRect = {(SCREEN_WIDTH - 700) / 2, (SCREEN_HEIGHT - 900) / 2, 700, 900};

    int menuX = menuRect.x;
    int menuY = menuRect.y;

    SDL_Rect characters[6] = {
        {menuX + 60,  menuY + 220,  190, 265}, // panda
        {menuX + 270, menuY + 200,  165, 320}, // giraff
        {menuX + 490, menuY + 210,  180, 290}, // räv
        {menuX + 50,  menuY + 520,  180, 270}, // björn
        {menuX + 260, menuY + 520,  150, 270}, // kanin
        {menuX + 445, menuY + 520,  230, 270}  // lejon
    };


    while (selected == -1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return -1;
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

        // RENDERING
        // Bakgrundsgräs
        for (int y = 0; y < SCREEN_HEIGHT; y += 64) {
            for (int x = 0; x < SCREEN_WIDTH; x += 64) {
                SDL_Rect dst = { x, y, 64, 64 };
                SDL_RenderCopy(renderer, grassTexture, NULL, &dst);
            }
        }

        // Mörk overlay ovanpå bakgrunden
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 80);
        SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);

        // Menybakgrund (karaktärsval)
        SDL_RenderCopy(renderer, menuTexture, NULL, &menuRect);

        // Rita outlines runt alla 6 klickrutor
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // vit färg
        // for (int i = 0; i < 6; i++) {
        //     SDL_RenderDrawRect(renderer, &characters[i]);
        // }

        // Vit outline runt karaktärer (om du har)
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // for (int i = 0; i < 6; i++) {
        //     SDL_RenderDrawRect(renderer, &characters[i]);
        // }

        // 🔲 Svart tjock outline runt menyboxen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        int thickness = 4;
        for (int i = 0; i < thickness; i++) {
            SDL_Rect outline = { menuRect.x - i, menuRect.y - i, menuRect.w + 2 * i, menuRect.h + 2 * i };
            SDL_RenderDrawRect(renderer, &outline);
        }

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
    TTF_Quit();
    SDLNet_Quit();
}