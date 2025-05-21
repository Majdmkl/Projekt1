#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "Map.h"
#include "Bullet.h"
#include "Character.h"
#include "Network.h"
#include "config.h"
#include "Text.h"

void initSDL();
bool initNetwork();
void cleanupNetwork();
SDL_Window* createWindow();
int mainMenu(SDL_Renderer* renderer);
void waitingRoom(SDL_Renderer* renderer);
bool connectToServer(const char* serverIP);
int selectCharacter(SDL_Renderer* renderer);
char* connectionScreen(SDL_Renderer* renderer);
SDL_Renderer* createRenderer(SDL_Window* window);
void endScreen(SDL_Renderer* renderer, bool won);
void sendPlayerData(Character* player, int action);
void gameLoop(SDL_Renderer* renderer, Character* player);
void cleanup(SDL_Window* window, SDL_Renderer* renderer);
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath);
Character* createSelectedCharacter(SDL_Renderer* renderer, int selected);

UDPsocket clientSocket;
UDPpacket *sendPacket, *receivePacket;
IPaddress serverIP;

int playerID = -1;
ServerData serverData;
bool connected = false;
SDL_Texture* mapTexture = NULL;

Mix_Music *bgMusic = NULL;
Mix_Chunk *shootSound = NULL, *hitSound = NULL, *buttonSound = NULL, *selectSound = NULL, *deliverSound = NULL;

int main(int argc, char* argv[]) {
    initSDL();

    if (!initNetwork()) { SDL_Log("Network initialization failed!"); return 1; }

    shootSound = Mix_LoadWAV("lib/assets/sounds/shoot.wav");
    hitSound = Mix_LoadWAV("lib/assets/sounds/hit.wav");

    bgMusic = Mix_LoadMUS("lib/assets/sounds/bg_music.mp3");
    buttonSound = Mix_LoadWAV("lib/assets/sounds/Button_press.wav");
    selectSound = Mix_LoadWAV("lib/assets/sounds/character_select.wav");
    deliverSound = Mix_LoadWAV("lib/assets/sounds/deliver.wav");
    Mix_PlayMusic(bgMusic, -1);

    SDL_Window* window = createWindow();
    SDL_Renderer* renderer = createRenderer(window);

    mapTexture = loadTexture(renderer, "lib/assets/images/ui/MapNew.png");

    int selected = -1;
    char *ip = NULL;
    int menuSelection = mainMenu(renderer);

    switch (menuSelection) {
    //start button = 0
    case 0: ip = (argc > 1) ? argv[1] : "127.0.0.1"; break;
    //connect button = 1
    case 1: ip = connectionScreen(renderer); break;
    //exit button = 2: ;
    case 2:
        SDL_Delay(100);
        cleanup(window, renderer);
        cleanupNetwork();
        return 0;
    default: break;
    }

    if (ip && connectToServer(ip)) selected = selectCharacter(renderer);

    Character* player = createSelectedCharacter(renderer, selected);

    ClientData initialData = {0};
    initialData.playerNumber = selected;
    initialData.command[0] = CONNECTING;
    initialData.animals.type = selected;

    memcpy(sendPacket->data, &initialData, sizeof(ClientData));
    sendPacket->len = sizeof(ClientData);
    SDLNet_UDP_Send(clientSocket, -1, sendPacket);

    Uint32 startTime = SDL_GetTicks();
    Uint32 lastTimeSent = startTime;
    while (SDL_GetTicks() - startTime < CONNECTION_TIMEOUT_MS) {
        if (SDLNet_UDP_Recv(clientSocket, receivePacket)) {
            memcpy(&serverData, receivePacket->data, sizeof(ServerData));

            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (serverData.slotsTaken[i] &&  serverData.animals[i].type == selected) {
                    playerID = i;
                    connected = true;
                    break;
                }
            }
            if (connected) break;
        }

        Uint32 now = SDL_GetTicks();
        if (now - lastTimeSent > NETWORK_SEND_INTERVAL_MS)
            if (sendPacket) { SDLNet_UDP_Send(clientSocket, -1, sendPacket); lastTimeSent = now; }

        SDL_Delay(100);
    }

    waitingRoom(renderer);
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
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
}

bool initNetwork() {
    clientSocket = SDLNet_UDP_Open(0);
    sendPacket = SDLNet_AllocPacket(sizeof(ClientData));
    receivePacket = SDLNet_AllocPacket(sizeof(ServerData));
    return true;
}

void cleanupNetwork() {
    SDLNet_FreePacket(sendPacket);
    SDLNet_FreePacket(receivePacket);
    SDLNet_UDP_Close(clientSocket);
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

static void applyPlayerAction(ClientData* clientData, Character* player, int action) {
    switch (action) {
        case 1: clientData->command[1] = UP; break;
        case 2: clientData->command[2] = DOWN; break;
        case 3: clientData->command[3] = LEFT; break;
        case 4: clientData->command[4] = RIGHT; break;
        case 5:
            clientData->command[5] = FIRE;
            clientData->bulletStartX = getX(player) + CHARACTER_WIDTH/2.0f;
            clientData->bulletStartY = getY(player) + CHARACTER_HEIGHT/2.0f;
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            float ddx = mouseX - clientData->bulletStartX;
            float ddy = mouseY - clientData->bulletStartY;
            float mag = sqrtf(ddx*ddx + ddy*ddy);
            if (mag > 0.0f) {
                clientData->bulletDx = ddx/mag * BULLET_SPEED;
                clientData->bulletDy = ddy/mag * BULLET_SPEED;
            } else {
                clientData->bulletDx = 0;
                clientData->bulletDy = 0;
            }
            break;
    }
}

void sendPlayerData(Character* player, int action) {
    if (!connected) return;

    ClientData clientData = {0};
    clientData.playerNumber = playerID;
    clientData.animals.x = getX(player);
    clientData.animals.y = getY(player);

    applyPlayerAction(&clientData, player, action);

    memcpy(sendPacket->data, &clientData, sizeof(ClientData));
    sendPacket->len = sizeof(ClientData);
    SDLNet_UDP_Send(clientSocket, -1, sendPacket);
}

SDL_Window* createWindow() {return SDL_CreateWindow("Safari Delivery", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0); }

SDL_Renderer* createRenderer(SDL_Window* window) { return SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); }

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = IMG_Load(filePath);
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
    SDL_Rect possibleHouses[] = {
        {153, 42, 166, 172},    // Bakery
        {74, 480, 153, 143},    // School (Blue house)
        {327, 755, 156, 155},   // Cafe (Yellow house)
        {1082, 768, 169, 140},  // Garden (Red house)
        {1165, 445, 169, 144},  // Blue house
        {1400, 116, 178, 158},  // Gym (White house)
        {1524, 512, 159, 139},  // Green house
        {1584, 765, 144, 122},  // Black house
    };

    SDL_Rect targets[3]; bool delivered[3] = {false}; int deliveriesRemaining = 3;
    srand(SDL_GetTicks());
    int houseCount = sizeof(possibleHouses)/sizeof(SDL_Rect);
    int ind [houseCount];
    for(int i = 0; i < houseCount; i++) ind[i] = i;

    for (int i = 0; i < 3; i++) {
        int j = i + rand() % (houseCount - i);
        int temp = ind[i];
        ind[i] = ind[j];
        ind[j] = temp;
    }

    for (int i = 0; i < 3; i++) targets[i] = possibleHouses[ind[i]];

    Bullet* bullets[MAX_BULLETS] = {NULL};
    int bulletCount = 0;

    Character* otherPlayers[MAX_PLAYERS] = {NULL};
    bool playerActive[MAX_PLAYERS] = {false};

    SDL_Texture* packageIcon = IMG_LoadTexture(renderer, "lib/assets/images/character/weapons/package1.png");
    SDL_SetTextureBlendMode(packageIcon, SDL_BLENDMODE_BLEND);
    SDL_Texture* housePackageIcon = IMG_LoadTexture(renderer, "lib/assets/images/character/weapons/Zone.png");
    SDL_SetTextureBlendMode(housePackageIcon, SDL_BLENDMODE_BLEND);

    setCharacterPackageIcon(player, packageIcon);
    setPackageCount(player, deliveriesRemaining);

    SDL_Event event;
    bool running = true;
    bool gameOver = false;
    bool spectating = false;
    float deathX = 0, deathY = 0;
    Uint32 lastNetworkUpdate = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (i != playerID && serverData.slotsTaken[i]) {
            otherPlayers[i] = createCharacter(renderer, serverData.animals[i].type);
            if (otherPlayers[i]) {
                setPosition(otherPlayers[i], serverData.animals[i].x, serverData.animals[i].y);
                setPackageCount(otherPlayers[i], serverData.animals[i].packages);
                setCharacterPackageIcon(otherPlayers[i], packageIcon);
                playerActive[i] = true;
            }
        }
    }

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && !spectating) {
                if (gameOver) break;
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                float startX = getX(player) + CHARACTER_WIDTH / 2.0f;
                float startY = getY(player) + CHARACTER_HEIGHT / 2.0f;
                float dx = mouseX - startX, dy = mouseY - startY;;
                float mag = sqrtf(dx * dx + dy * dy);
                if (mag>0) { dx = dx/mag * BULLET_SPEED; dy = dy/mag * BULLET_SPEED; }
                else { dx = BULLET_SPEED; dy = 0; }

                if (bulletCount < MAX_BULLETS) bullets[bulletCount++] = createBullet(renderer, startX, startY, dx, dy, playerID);

                sendPlayerData(player, 5);
                Mix_PlayChannel(-1, shootSound, 0);
            }
            if (event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_e && player) {
                SDL_Rect pRect={ (int)getX(player), (int)getY(player), CHARACTER_WIDTH, CHARACTER_HEIGHT };
                for(int i=0;i<3;i++) if(!delivered[i] && SDL_HasIntersection(&pRect, &targets[i])) {
                    delivered[i]=true; deliveriesRemaining--; setPackageCount(player, deliveriesRemaining);
                    Mix_PlayChannel(-1, deliverSound, 0);
                }
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        float moveX = 0, moveY = 0;
        bool actionSent = 0;

        if (!spectating) {
            if (gameOver) { moveX = moveY = 0; }
            if (keys[SDL_SCANCODE_W]) {
                moveY -= MOVE_SPEED;
                turnUp(player);
                if (!actionSent) { sendPlayerData(player, 1); actionSent = true; }
            }
            if (keys[SDL_SCANCODE_S]) {
                moveY += MOVE_SPEED;
                turnDown(player);
                if (!actionSent) { sendPlayerData(player, 2); actionSent = true; }
            }
            if (keys[SDL_SCANCODE_A]) {
                moveX -= MOVE_SPEED;
                turnLeft(player);
                if (!actionSent) { sendPlayerData(player, 3); actionSent = true; }
            }
            if (keys[SDL_SCANCODE_D]) {
                moveX += MOVE_SPEED;
                turnRight(player);
                if (!actionSent) { sendPlayerData(player, 4); actionSent = true; }
            }
        }

        if (moveX != 0 && moveY != 0) {
            float diagSpeed = MOVE_SPEED / 1.4142f;
            moveX = (moveX > 0) ? diagSpeed : -diagSpeed;
            moveY = (moveY > 0) ? diagSpeed : -diagSpeed;
        }

        if (!spectating) {
            float prevPlayerX = getX(player);
            float prevPlayerY = getY(player);
            moveCharacter(player, moveX, moveY, walls, MAX_WALLS);

            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (i != playerID && playerActive[i] && otherPlayers[i]) {
                    SDL_Rect pRect = { getX(player), getY(player), CHARACTER_WIDTH, CHARACTER_HEIGHT };
                    SDL_Rect oRect = { getX(otherPlayers[i]), getY(otherPlayers[i]), CHARACTER_WIDTH, CHARACTER_HEIGHT };
                    if (SDL_HasIntersection(&pRect, &oRect)) { setPosition(player, prevPlayerX, prevPlayerY); break; }
                }
            }
            updateCharacterAnimation(player, SDL_GetTicks());
        }

        Uint32 now = SDL_GetTicks();
        if (now - lastNetworkUpdate > NETWORK_TICK_MS) {
            bool gotData = false;
            while (SDLNet_UDP_Recv(clientSocket, receivePacket)) {
                memcpy(&serverData, receivePacket->data, sizeof(ServerData));
                gotData = true;
            }
            if (gotData) {
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    if (i != playerID && serverData.slotsTaken[i]) {
                        if (!playerActive[i] || !otherPlayers[i]) {
                            otherPlayers[i] = createCharacter(renderer, serverData.animals[i].type);
                            if (otherPlayers[i]) setCharacterPackageIcon(otherPlayers[i], packageIcon);
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
                            setPackageCount(otherPlayers[i], serverData.animals[i].packages);
                            updateCharacterAnimation(otherPlayers[i], SDL_GetTicks());
                            int oldHP = getPlayerHP(otherPlayers[i]);
                            int srvHP = serverData.animals[i].health;
                            if (srvHP < oldHP) Mix_PlayChannel(-1, hitSound, 0);
                            while (getPlayerHP(otherPlayers[i]) > srvHP) decreaseHealth(otherPlayers[i]);
                        }
                    } else if (i != playerID && !serverData.slotsTaken[i] && playerActive[i]) {
                        if (otherPlayers[i]) destroyCharacter(otherPlayers[i]);
                        otherPlayers[i] = NULL;
                        playerActive[i] = false;
                    }
                }

                if (serverData.slotsTaken[playerID]) {
                    int oldHP = getPlayerHP(player);
                    int srvHP = serverData.animals[playerID].health;
                    if (srvHP < oldHP) Mix_PlayChannel(-1, hitSound, 0);
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
        for (int i = 0; i < MAX_PLAYERS; ++i)
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
        SDL_RenderCopy(renderer, mapTexture, NULL, NULL);

        if (player && getPlayerHP(player) > 0) {
            renderCharacter(player, renderer);
            healthBar(player, renderer);
        } else if (spectating) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect deathRect = { (int)deathX, (int)deathY, CHARACTER_WIDTH, CHARACTER_HEIGHT };
            SDL_RenderFillRect(renderer, &deathRect);
        }

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (i != playerID && playerActive[i] && otherPlayers[i]) {
                if (getPlayerHP(otherPlayers[i]) > 0) renderCharacter(otherPlayers[i], renderer);
                healthBar(otherPlayers[i], renderer);
            }
        }
        for (int i = 0; i < bulletCount; i++) drawBullet(bullets[i], renderer);
        for(int i=0;i<3;i++) {
            if(!delivered[i]) {
                SDL_Rect h = targets[i];
                SDL_Rect iconDest = { h.x + (h.w - 64)/2, h.y - 64, 64, 64 };
                SDL_RenderCopy(renderer, housePackageIcon, NULL, &iconDest);
            }
        }

        int aliveCount=0; for(int i=0;i<MAX_PLAYERS;i++) if(i==playerID? getPlayerHP(player)>0 : playerActive[i]) aliveCount++;
        if(deliveriesRemaining == 0 || aliveCount <= 1) {
            gameOver = true;
            bool won = player && (deliveriesRemaining == 0 || (aliveCount == 1 && getPlayerHP(player) > 0));
            endScreen(renderer, won); // needs improvements -- change to image
            SDL_Event event;
            while (SDL_WaitEvent(&event)) if (event.type == SDL_QUIT) break;

            running = false;
            continue;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(FRAME_DELAY_MS);
    }

    for (int i = 0; i < bulletCount; i++) destroyBullet(bullets[i]);
    for (int i = 0; i < MAX_PLAYERS; i++) if (i != playerID && playerActive[i]) destroyCharacter(otherPlayers[i]);

}
// needs imrovements -- change to image
void endScreen(SDL_Renderer* renderer, bool won) {
    TTF_Font* font = TTF_OpenFont("lib/assets/fonts/PressStart2P-Regular.ttf", 48);
    SDL_Color white={255,255,255,255}, red={255,0,0,255};
    SDL_Color green={0,255,0,255};
    char* msg = won? "You Won" : "You Lost";
    Text* text = createText(renderer, won?green.r:red.r, won?green.g:red.g, won?green.b:red.b, font, msg, SCREEN_WIDTH/2, won?SCREEN_HEIGHT/2:50);
    drawTextCentered(text, SCREEN_WIDTH/2, won?SCREEN_HEIGHT/2:50);
    SDL_RenderPresent(renderer);
}

static void tileGrass(SDL_Renderer* renderer, SDL_Texture* grass) {
    for (int y = 0; y < SCREEN_HEIGHT; y += 64)
        for (int x = 0; x < SCREEN_WIDTH; x += 64) {
            SDL_Rect dst = { x, y, 64, 64 };
            SDL_RenderCopy(renderer, grass, NULL, &dst);
        }
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

                if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &inputBox)) {
                    typingActive = true;
                    SDL_StartTextInput();
                    Mix_PlayChannel(-1, buttonSound, 0);
                }
                else { typingActive = false; SDL_StopTextInput(); }

                if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &closeBtn)) {
                    ip[0] = '\0';
                    typingActive = false;
                    SDL_StopTextInput();
                }
            }
            else if (typingActive && event.type == SDL_TEXTINPUT) {
                strncat(ip, event.text.text, sizeof(ip) - strlen(ip) - 1);
                Mix_PlayChannel(-1, buttonSound, 0);
            }
            else if (typingActive && event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(ip) > 0) ip[strlen(ip) - 1] = '\0';
                if (event.key.keysym.sym == SDLK_RETURN) {
                    Mix_PlayChannel(-1, buttonSound, 0);
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
        tileGrass(renderer, grassTexture);

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
            Text *ipText = createText(renderer, 20, 120, 20, font, ip, inputBox.x + inputBox.w / 2, inputBox.y + inputBox.h / 2);
            drawTextCentered(ipText, inputBox.x + inputBox.w / 2, inputBox.y + inputBox.h / 2);
            destroyText(ipText);
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
    TTF_Font* font = TTF_OpenFont("lib/assets/fonts/PressStart2P-Regular.ttf", 24);
    SDL_Event event;
    SDL_Rect menuRect = {(SCREEN_WIDTH - 700) / 2, (SCREEN_HEIGHT - 900) / 2, 690, 910};
    SDL_Rect continueBtn = { menuRect.x + 170, menuRect.y + 765, 390, 85 };
    bool pressed = false;
    Uint32 lastSent = SDL_GetTicks();

    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int x = event.button.x, y = event.button.y;
                if (SDL_PointInRect(&(SDL_Point){x, y}, &continueBtn)) {
                    Mix_PlayChannel(-1, buttonSound, 0);
                    pressed = true;
                }
            }
        }

        if (pressed && SDL_GetTicks() - lastSent > 200) {
            ClientData cd = {0};
            cd.playerNumber = playerID;
            cd.command[7] = CONTINUE;
            memcpy(sendPacket->data, &cd, sizeof(ClientData));
            sendPacket->len = sizeof(ClientData);
            SDLNet_UDP_Send(clientSocket, -1, sendPacket);
            lastSent = SDL_GetTicks();
        }

        while (SDLNet_UDP_Recv(clientSocket, receivePacket)) memcpy(&serverData, receivePacket->data, sizeof(ServerData));

        tileGrass(renderer, grassTexture);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 80);
        SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}; SDL_RenderFillRect(renderer, &overlay);
        SDL_RenderCopy(renderer, bgTexture, NULL, &menuRect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int i = 0; i < 4; i++) SDL_RenderDrawRect(renderer, &(SDL_Rect){ menuRect.x - i, menuRect.y - i, menuRect.w + 2*i, menuRect.h + 2*i });
        char buf[32]; sprintf(buf, "Ready: %d/%d", serverData.readyCount, serverData.numberOfPlayers);
        Text *readyText = createText(renderer, 255, 255, 255, font, buf, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
        drawTextCentered(readyText, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
        destroyText(readyText);
        SDL_RenderPresent(renderer);
        if (serverData.gameState == ONGOING) break;
        SDL_Delay(FRAME_DELAY_MS);
    }
    SDL_DestroyTexture(bgTexture); SDL_DestroyTexture(grassTexture); TTF_CloseFont(font);
    return;
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

                if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &startBtn)) { Mix_PlayChannel(-1, buttonSound, 0); selection = 0; }
                else if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &connectBtn)) { Mix_PlayChannel(-1, buttonSound, 0); selection = 1; }
                else if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &quitBtn))   { Mix_PlayChannel(-1, buttonSound, 0); selection = 2; }
            }
        }

        tileGrass(renderer, grassTexture);

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
                        Mix_PlayChannel(-1, selectSound, 0);
                        selected = i;
                        break;
                    }
                }
            }
        }

        // RENDERING
        // Bakgrundsgräs
        tileGrass(renderer, grassTexture);

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
    if (mapTexture) SDL_DestroyTexture(mapTexture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    cleanupNetwork();
    Mix_FreeChunk(shootSound);
    Mix_FreeChunk(hitSound);
    Mix_FreeChunk(buttonSound);
    Mix_FreeChunk(selectSound);
    Mix_FreeChunk(deliverSound);
    Mix_FreeMusic(bgMusic);
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
    IMG_Quit();
    TTF_Quit();
}