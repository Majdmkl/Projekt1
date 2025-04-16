#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 1750
#define SCREEN_HEIGHT 1000
#define CHARACTER_WIDTH 90
#define CHARACTER_HEIGHT 80
#define TILE_SIZE 128
#define MAP_WIDTH 14
#define MAP_HEIGHT 8
#define FRAME_COUNT 2
#define FRAME_DELAY 100


enum PlayerState {
    IDLE,
    WALKING_UP,
    WALKING_DOWN,
    WALKING_LEFT,
    WALKING_RIGHT
};

int map[MAP_HEIGHT][MAP_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

void initSDL();
SDL_Window* createWindow();
int selectCharacter(SDL_Renderer* renderer);
SDL_Renderer* createRenderer(SDL_Window* window);
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath);
void loadCharacterTextures(SDL_Renderer* renderer, int selected, SDL_Texture** textures);
void gameLoop(SDL_Renderer* renderer, SDL_Texture** tileTextures, SDL_Texture* treeTexture, SDL_Texture* cottageTexture, SDL_Texture** playerTextures);
void cleanup(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture** tileTextures, SDL_Texture* treeTexture, SDL_Texture** playerTextures);

int main(int argc, char* argv[]) {

    initSDL();

    SDL_Window* window = createWindow();
    SDL_Renderer* renderer = createRenderer(window);

    SDL_Texture* tileTextures[2];
    tileTextures[0] = loadTexture(renderer, "lib/assets/grass.png");
    tileTextures[1] = loadTexture(renderer, "lib/assets/water.png");

    SDL_Texture* treeTexture = loadTexture(renderer, "lib/assets/objects/Tree.png");
    SDL_Texture* cottageTexture = loadTexture(renderer, "lib/assets/objects/Cottage.png");
    //SDL_Texture* fountainTexture = loadTexture(renderer, "lib/assets/objects/Fountain.png");
    //SDL_Texture* bushTexture = loadTexture(renderer, "lib/assets/objects/Bush.png");
    //SDL_Texture* riverTexture = loadTexture(renderer, "lib/assets/objects/River.png");
    //SDL_Texture* bridgeTexture = loadTexture(renderer, "lib/assets/objects/Bridge.png");
    //SDL_Texture* mailboxTexture = loadTexture(renderer, "lib/assets/objects/Mailbox.png");
    //SDL_Texture* street_lampTexture = loadTexture(renderer, "lib/assets/objects/Street_lamp.png");

    int selected = selectCharacter(renderer);
    if (selected == -1) { cleanup(window, renderer, tileTextures, treeTexture, NULL); return 1; }

    SDL_Texture* playerTextures[5] = {NULL};
    loadCharacterTextures(renderer, selected, playerTextures);

    for (int i = 0; i < 5; i++) {
        if (!playerTextures[i]) {
            SDL_Log("Could not load all character sprites.");
            cleanup(window, renderer, tileTextures, treeTexture, playerTextures);
            cleanup(window, renderer, tileTextures, cottageTexture, playerTextures);
            return 1;
        }
    }

    gameLoop(renderer, tileTextures, treeTexture, cottageTexture, playerTextures);
    cleanup(window, renderer, tileTextures, treeTexture, playerTextures);

    gameLoop(renderer, tileTextures, treeTexture, cottageTexture, playerTextures);
    cleanup(window, renderer, tileTextures, cottageTexture, playerTextures);

    return 0;
}

void initSDL() { SDL_Init(SDL_INIT_VIDEO); IMG_Init(IMG_INIT_PNG); }
SDL_Window* createWindow() { return SDL_CreateWindow("COZY DELIVERY", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0); }
SDL_Renderer* createRenderer(SDL_Window* window) { return SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); }

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) { SDL_Log("Failed to load image: %s\n", IMG_GetError()); return NULL; }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void loadCharacterTextures(SDL_Renderer* renderer, int selected, SDL_Texture** textures) {
    const char* character = NULL;

    switch (selected) {
        case 0: character = "panda"; break;
        case 1: character = "giraffe"; break;
        case 2: character = "fox"; break;
        case 3: character = "bear"; break;
        case 4: character = "bunny"; break;
        case 5: character = "lion"; break;
        default: return;
    }

    char path[100];

    sprintf(path, "lib/assets/animal/%s/%s_right_walk_spritesheet.png", character, character);
    textures[0] = IMG_LoadTexture(renderer, path);

    sprintf(path, "lib/assets/animal/%s/%s_left_walk_spritesheet.png", character, character);
    textures[1] = IMG_LoadTexture(renderer, path);

    sprintf(path, "lib/assets/animal/%s/%s_front_walk_spritesheet.png", character, character);
    textures[2] = IMG_LoadTexture(renderer, path);

    sprintf(path, "lib/assets/animal/%s/%s_back_walk_spritesheet.png", character, character);
    textures[3] = IMG_LoadTexture(renderer, path);

    sprintf(path, "lib/assets/animal/%s/%s_front.png", character, character);
    textures[4] = IMG_LoadTexture(renderer, path);
}

void gameLoop(SDL_Renderer* renderer, SDL_Texture** tileTextures, SDL_Texture* treeTexture,
              SDL_Texture* cottageTexture, SDL_Texture** playerTextures) {

    SDL_Texture *walkRight = playerTextures[0];
    SDL_Texture *walkLeft  = playerTextures[1];
    SDL_Texture *walkDown  = playerTextures[2];
    SDL_Texture *walkUp    = playerTextures[3];
    SDL_Texture *idleFront = playerTextures[4];

    float speed = 5;
    int playerX = 500, playerY = 500, frame = 0;;
    Uint32 lastFrameTime = SDL_GetTicks();

    SDL_Event event;
    bool running = true;

    while (running) {
        while (SDL_PollEvent(&event)) if(event.type == SDL_QUIT) running = false;

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        float prevX = playerX, prevY = playerY;
        float moveX = 0, moveY = 0;
        enum PlayerState playerState = IDLE;

        if (keys[SDL_SCANCODE_W]) {
            moveY -= speed;
            playerState = WALKING_UP;
        }
        if (keys[SDL_SCANCODE_S]) {
            moveY += speed;
            playerState = WALKING_DOWN;
        }
        if (keys[SDL_SCANCODE_A]) {
            moveX -= speed;
            playerState = WALKING_LEFT;
        }
        if (keys[SDL_SCANCODE_D]) {
            moveX += speed;
            playerState = WALKING_RIGHT;
        }

        if (moveX != 0 && moveY != 0) {
            float diagSpeed = speed / 1.4142f;
            moveX = (moveX > 0) ? diagSpeed : -diagSpeed;
            moveY = (moveY > 0) ? diagSpeed : -diagSpeed;
        }

        playerX += moveX;
        playerY += moveY;
        // collision needs improvement, this is just a temp fix
        int tileX = playerX / TILE_SIZE;
        int tileY = playerY / TILE_SIZE;
        if (tileX < 0 || tileX >= MAP_WIDTH || tileY < 0 || tileY >= MAP_HEIGHT || map[tileY][tileX] == 1) {
            playerX = prevX;
            playerY = prevY;
            playerState = IDLE;
        }

        Uint32 currentTime = SDL_GetTicks();
        bool isMoving = (playerState != IDLE);
        if (isMoving && currentTime > lastFrameTime + FRAME_DELAY) {
            frame = (frame + 1) % FRAME_COUNT;
            lastFrameTime = currentTime;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                SDL_Rect dst = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderCopy(renderer, tileTextures[map[y][x]], NULL, &dst);
            }
        }

        SDL_Rect treeRect = { 300, 300, 128, 128 };
        SDL_Rect cottageRect = { 600, 600, 145, 130 };
        SDL_RenderCopy(renderer, treeTexture, NULL, &treeRect);
        SDL_RenderCopy(renderer, cottageTexture, NULL, &cottageRect);

        SDL_Rect srcRect = { frame * 64, 0, 64, 90 };
        SDL_Rect destRect = { playerX, playerY, CHARACTER_WIDTH, CHARACTER_HEIGHT + 16 };

        switch (playerState) {
            case WALKING_DOWN:
                SDL_RenderCopy(renderer, walkDown, &srcRect, &destRect);
                break;
            case WALKING_UP:
                SDL_RenderCopy(renderer, walkUp, &srcRect, &destRect);
                break;
            case WALKING_LEFT:
                SDL_RenderCopy(renderer, walkLeft, &srcRect, &destRect);
                break;
            case WALKING_RIGHT:
                SDL_RenderCopy(renderer, walkRight, &srcRect, &destRect);
                break;
            case IDLE:
            default:
                SDL_RenderCopy(renderer, idleFront, NULL, &destRect);
                break;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ca 60 pfs
    }
}


int selectCharacter(SDL_Renderer* renderer) {
    SDL_Texture* menuTexture = loadTexture(renderer, "lib/assets/meny.png");
    SDL_Texture* grassTexture = loadTexture(renderer, "lib/assets/grass.png");

    SDL_Event event;
    int selected = -1;

    SDL_Rect menuRect = {
        (SCREEN_WIDTH - 384) / 2,
        (SCREEN_HEIGHT - 384) / 2,
        384, 384
    };

    int menuX = menuRect.x;
    int menuY = menuRect.y;

    SDL_Rect characters[6] = {
        {menuX + 32,  menuY + 140,  64, 64},
        {menuX + 185, menuY + 140,  64, 64},
        {menuX + 288, menuY + 150,  64, 64},
        {menuX + 43,  menuY + 280,  64, 64},
        {menuX + 160, menuY + 270,  64, 64},
        {menuX + 280, menuY + 280,  64, 64}
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

        for (int y = 0; y < SCREEN_HEIGHT; y += 64) {
            for (int x = 0; x < SCREEN_WIDTH; x += 64) {
                SDL_Rect dst = { x, y, 64, 64 };
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

void cleanup(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture** tileTextures, SDL_Texture* treeTexture, SDL_Texture** playerTextures) {

    if (playerTextures) for (int i = 0; i < 5; i++) if (playerTextures[i]) SDL_DestroyTexture(playerTextures[i]);
    if (tileTextures) for (int i = 0; i < 2; i++) if (tileTextures[i]) SDL_DestroyTexture(tileTextures[i]);
    if (treeTexture) SDL_DestroyTexture(treeTexture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();
}