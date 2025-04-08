#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define TILE_SIZE 128
#define MAP_WIDTH 10
#define MAP_HEIGHT 6
#define FRAME_WIDTH 32
#define FRAME_HEIGHT 32
#define FRAME_DELAY 8
#define MAX_FRAMES 4

int map[MAP_HEIGHT][MAP_WIDTH] = {
    {0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    {1, 1, 0, 1, 0, 0, 0, 0, 0, 0},
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
    for (int i = 0; i < 2; ++i) {
        tileTextures[i] = SDL_CreateTextureFromSurface(renderer, tileSurfaces[i]);
        SDL_FreeSurface(tileSurfaces[i]);
    }

    SDL_Surface* bunnySheet = IMG_Load("assets/bunny_sprite_sheet.png");
    SDL_Texture* bunnyTexture = SDL_CreateTextureFromSurface(renderer, bunnySheet);
    SDL_FreeSurface(bunnySheet);

    SDL_Rect srcRect = {0, 0, FRAME_WIDTH, FRAME_HEIGHT};
    SDL_Rect dest = {0, 0, 64, 64};
    int direction = 0; // 0 = down, 1 = left, 2 = right, 3 = up
    int frame = 0;
    int frameTime = 0;

    int playerX = 0;
    int playerY = 0;
    int speed = 4;

    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        int prevX = playerX;
        int prevY = playerY;
        int moving = 0;

        if (keys[SDL_SCANCODE_UP]) {
            direction = 3;
            playerY -= speed;
            moving = 1;
        } else if (keys[SDL_SCANCODE_DOWN]) {
            direction = 0;
            playerY += speed;
            moving = 1;
        } else if (keys[SDL_SCANCODE_LEFT]) {
            direction = 1;
            playerX -= speed;
            moving = 1;
        } else if (keys[SDL_SCANCODE_RIGHT]) {
            direction = 2;
            playerX += speed;
            moving = 1;
        }

        int tileX = playerX / TILE_SIZE;
        int tileY = playerY / TILE_SIZE;

        if (tileX < 0 || tileX >= MAP_WIDTH || tileY < 0 || tileY >= MAP_HEIGHT || map[tileY][tileX] == 1) {
            playerX = prevX;
            playerY = prevY;
            moving = 0;
        }

        if (moving) {
            frameTime++;
            if (frameTime >= FRAME_DELAY) {
                frame = (frame + 1) % MAX_FRAMES;
                frameTime = 0;
            }
        } else {
            frame = 0;
        }

        srcRect.x = frame * FRAME_WIDTH;
        srcRect.y = direction * FRAME_HEIGHT;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                SDL_Rect dst = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderCopy(renderer, tileTextures[map[y][x]], NULL, &dst);
            }
        }

        dest.x = playerX;
        dest.y = playerY;
        SDL_RenderCopy(renderer, bunnyTexture, &srcRect, &dest);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(bunnyTexture);
    for (int i = 0; i < 2; ++i) SDL_DestroyTexture(tileTextures[i]);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
