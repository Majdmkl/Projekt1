#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define TILE_SIZE 128
#define MAP_WIDTH 10
#define MAP_HEIGHT 6
#define FRAME_COUNT 2
#define FRAME_DELAY 100
#define SPRITE_WIDTH 64
#define SPRITE_HEIGHT 64

int map[MAP_HEIGHT][MAP_WIDTH] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return -1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_Log("IMG_Init Error: %s", IMG_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Cute Bunny", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture* tileTextures[2];
    tileTextures[0] = IMG_LoadTexture(renderer, "assets/grass.png");
    tileTextures[1] = IMG_LoadTexture(renderer, "assets/water.png");

    if (!tileTextures[0] || !tileTextures[1]) {
        SDL_Log("Failed to load tiles: %s", SDL_GetError());
        for (int i = 0; i < 2; ++i) if (tileTextures[i]) SDL_DestroyTexture(tileTextures[i]);

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture* bunnyWalkRight = IMG_LoadTexture(renderer, "assets/panda_spritesheets/panda_walk_right.png");
    SDL_Texture* bunnyWalkLeft = IMG_LoadTexture(renderer, "assets/panda_spritesheets/panda_walk_left.png");

    if (!bunnyWalkRight || !bunnyWalkLeft) {
        SDL_Log("Could not load spritesheets: %s", SDL_GetError());
        for (int i = 0; i < 2; ++i) SDL_DestroyTexture(tileTextures[i]);

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    int frame = 0;
    Uint32 lastFrameTime = SDL_GetTicks();

    int playerX = 100, playerY = 100;
    int speed = 10;
    bool facingLeft = false;

    SDL_Event event;
    bool isRunning = true;

    while (isRunning) {
        while (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) isRunning = false;

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        int prevX = playerX;
        int prevY = playerY;
        bool isMoving = false;

        int moveX = 0;
        int moveY = 0;

        if (keys[SDL_SCANCODE_W]) {
            moveY -= speed;
            isMoving = true;
        } else if (keys[SDL_SCANCODE_S]) {
            moveY += speed;
            isMoving = true;
        }

        if (keys[SDL_SCANCODE_A]) {
            moveX -= speed;
            facingLeft = true;
            isMoving = true;
        } else if (keys[SDL_SCANCODE_D]) {
            moveX += speed;
            facingLeft = false;
            isMoving = true;
        }

        playerX += moveX;
        playerY += moveY;

        int tileX = playerX / TILE_SIZE;
        int tileY = playerY / TILE_SIZE;

        if (tileX < 0 || tileX >= MAP_WIDTH || tileY < 0 || tileY >= MAP_HEIGHT || map[tileY][tileX] == 1) {
            playerX = prevX;
            playerY = prevY;
        }

        Uint32 currentTime = SDL_GetTicks();
        if (isMoving && currentTime - lastFrameTime > FRAME_DELAY) {
            frame = (frame + 1) % FRAME_COUNT;
            lastFrameTime = currentTime;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < MAP_HEIGHT; ++y) {
            for (int x = 0; x < MAP_WIDTH; ++x) {
                SDL_Rect dst = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderCopy(renderer, tileTextures[map[y][x]], NULL, &dst);
            }
        }

        SDL_Rect srcRect = { frame * SPRITE_WIDTH, 0, SPRITE_WIDTH, SPRITE_HEIGHT };
        SDL_Rect destRect = { playerX, playerY, SPRITE_WIDTH, SPRITE_HEIGHT };

        SDL_RenderCopy(renderer, facingLeft ? bunnyWalkLeft : bunnyWalkRight, &srcRect, &destRect);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    for (int i = 0; i < 2; ++i) SDL_DestroyTexture(tileTextures[i]);

    SDL_DestroyTexture(bunnyWalkRight);
    SDL_DestroyTexture(bunnyWalkLeft);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}