#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define TILE_SIZE 128
#define MAP_WIDTH 10
#define MAP_HEIGHT 6
#define SCALE_FACTOR 1
#define FRAME_COUNT 2
#define FRAME_DELAY 100

int map[MAP_HEIGHT][MAP_WIDTH] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Cute Bunny", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface* tileSurfaces[2];
    tileSurfaces[0] = IMG_Load("assets/grass.png");
    tileSurfaces[1] = IMG_Load("assets/water.png");

    SDL_Texture* tileTextures[2];
    for (int i = 0; i < 2; ++i){
        tileTextures[i] = SDL_CreateTextureFromSurface(renderer, tileSurfaces[i]);
        SDL_FreeSurface(tileSurfaces[i]);
    }

    SDL_Texture* bunnyWalkRight = IMG_LoadTexture(renderer, "assets/panda_spritesheets/panda_walk_right.png");
    SDL_Texture* bunnyWalkLeft = IMG_LoadTexture(renderer, "assets/panda_spritesheets/panda_walk_left.png");

    if (!bunnyWalkRight || !bunnyWalkLeft) {
        printf("Kunde inte ladda spritesheets: %s\n", SDL_GetError());
        return 1;
    }

    int frame = 0;
    Uint32 lastFrameTime = SDL_GetTicks();

    int playerX = 100;
    int playerY = 100;
    int speed = 10;
    int facingLeft = 0;

    SDL_Event event;
    int running = 1;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) running = 0;
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        int prevX = playerX;
        int prevY = playerY;
        int moving = 0;

        int moveX = 0;
        int moveY = 0;

        if (keys[SDL_SCANCODE_W])
        {
            moveY -= speed;
            moving = 1;
        } else if (keys[SDL_SCANCODE_S])
        {
            moveY += speed;
            moving = 1;
        }

        if (keys[SDL_SCANCODE_A])
        {
            moveX -= speed;
            facingLeft = 1;
            moving = 1;
        } else if (keys[SDL_SCANCODE_D])
        {
            moveX += speed;
            facingLeft = 0;
            moving = 1;
        }

        playerX += moveX;
        playerY += moveY;

        int tileX = playerX / TILE_SIZE;
        int tileY = playerY / TILE_SIZE;
        if (tileX < 0 || tileX >= MAP_WIDTH || tileY < 0 || tileY >= MAP_HEIGHT || map[tileY][tileX] == 1)
        {
            playerX = prevX;
            playerY = prevY;
        }

        Uint32 currentTime = SDL_GetTicks();
        if (moving && currentTime > lastFrameTime + FRAME_DELAY)
        {
            frame = (frame + 1) % FRAME_COUNT;
            lastFrameTime = currentTime;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                SDL_Rect dst = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderCopy(renderer, tileTextures[map[y][x]], NULL, &dst);
            }
        }

        SDL_Rect srcRect = { frame * 64, 0, 64, 64 };
        SDL_Rect destRect = { playerX, playerY, 64, 64 };

        if (facingLeft) SDL_RenderCopy(renderer, bunnyWalkLeft, &srcRect, &destRect);
        else SDL_RenderCopy(renderer, bunnyWalkRight, &srcRect, &destRect);

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