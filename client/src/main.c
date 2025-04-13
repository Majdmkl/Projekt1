#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 1750
#define SCREEN_HEIGHT 1000
#define TILE_SIZE 128
#define MAP_WIDTH 14
#define MAP_HEIGHT 8
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

int selectCharacter(SDL_Renderer* renderer);
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath);

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("COZY DELIVERY", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Surface* tileSurfaces[2];
    tileSurfaces[0] = IMG_Load("lib/assets/grass.png");
    tileSurfaces[1] = IMG_Load("lib/assets/water.png");

    SDL_Texture* tileTextures[2];
    for (int i = 0; i < 2; ++i) {
        tileTextures[i] = SDL_CreateTextureFromSurface(renderer, tileSurfaces[i]);
        SDL_FreeSurface(tileSurfaces[i]);
    }

    SDL_Texture *walkRight, *walkLeft, *walkDown, *walkUp, *idleFront;

    SDL_Texture* treeTexture = loadTexture(renderer, "lib/assets/tree.png");

    int selected = selectCharacter(renderer);

    switch (selected) {
        case 0:
            walkRight = IMG_LoadTexture(renderer, "lib/assets/animal/panda/panda_right_walk_spritesheet.png");
            walkLeft  = IMG_LoadTexture(renderer, "lib/assets/animal/panda/panda_left_walk_spritesheet.png");
            walkDown  = IMG_LoadTexture(renderer, "lib/assets/animal/panda/panda_front_walk_spritesheet.png");
            walkUp    = IMG_LoadTexture(renderer, "lib/assets/animal/panda/panda_back_walk_spritesheet.png");
            idleFront = IMG_LoadTexture(renderer, "lib/assets/animal/panda/panda_front.png");
            break;
        case 1:
            walkRight = IMG_LoadTexture(renderer, "lib/assets/animal/giraffe/giraffe_right_walk_spritesheet.png");
            walkLeft  = IMG_LoadTexture(renderer, "lib/assets/animal/giraffe/giraffe_left_walk_spritesheet.png");
            walkDown  = IMG_LoadTexture(renderer, "lib/assets/animal/giraffe/giraffe_front_walk_spritesheet.png");
            walkUp    = IMG_LoadTexture(renderer, "lib/assets/animal/giraffe/giraffe_back_walk_spritesheet.png");
            idleFront = IMG_LoadTexture(renderer, "lib/assets/animal/giraffe/giraffe_front.png");
            break;
        case 2:
            walkRight = IMG_LoadTexture(renderer, "lib/assets/animal/fox/fox_right_walk_spritesheet.png");
            walkLeft  = IMG_LoadTexture(renderer, "lib/assets/animal/fox/fox_left_walk_spritesheet.png");
            walkDown  = IMG_LoadTexture(renderer, "lib/assets/animal/fox/fox_front_walk_spritesheet.png");
            walkUp    = IMG_LoadTexture(renderer, "lib/assets/animal/fox/fox_back_walk_spritesheet.png");
            idleFront = IMG_LoadTexture(renderer, "lib/assets/animal/fox/fox_front.png");
            break;
        case 3:
            walkRight = IMG_LoadTexture(renderer, "lib/assets/animal/bear/bear_right_walk_spritesheet.png");
            walkLeft  = IMG_LoadTexture(renderer, "lib/assets/animal/bear/bear_left_walk_spritesheet.png");
            walkDown  = IMG_LoadTexture(renderer, "lib/assets/animal/bear/bear_front_walk_spritesheet.png");
            walkUp    = IMG_LoadTexture(renderer, "lib/assets/animal/bear/bear_back_walk_spritesheet.png");
            idleFront = IMG_LoadTexture(renderer, "lib/assets/animal/bear/bear_front.png");
            break;
        case 4:
            walkRight = IMG_LoadTexture(renderer, "lib/assets/animal/bunny/bunny_right_walk_spritesheet.png");
            walkLeft  = IMG_LoadTexture(renderer, "lib/assets/animal/bunny/bunny_left_walk_spritesheet.png");
            walkDown  = IMG_LoadTexture(renderer, "lib/assets/animal/bunny/bunny_front_walk_spritesheet.png");
            walkUp    = IMG_LoadTexture(renderer, "lib/assets/animal/bunny/bunny_back_walk_spritesheet.png");
            idleFront = IMG_LoadTexture(renderer, "lib/assets/animal/bunny/bunny_front.png");
            break;
        case 5:
            walkRight = IMG_LoadTexture(renderer, "lib/assets/animal/lion/lion_right_walk_spritesheet.png");
            walkLeft  = IMG_LoadTexture(renderer, "lib/assets/animal/lion/lion_left_walk_spritesheet.png");
            walkDown  = IMG_LoadTexture(renderer, "lib/assets/animal/lion/lion_front_walk_spritesheet.png");
            walkUp    = IMG_LoadTexture(renderer, "lib/assets/animal/lion/lion_back_walk_spritesheet.png");
            idleFront = IMG_LoadTexture(renderer, "lib/assets/animal/lion/lion_front.png");
            break;
        default:
            printf("Ogiltigt val av karaktär.\n");
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            IMG_Quit();
            SDL_Quit();
            return 1;
    }

    if (!walkRight || !walkLeft || !walkDown || !walkUp || !idleFront) { SDL_Log("Kunde inte ladda sprites."); return 1;}

    int playerX = 100, playerY = 100;
    int speed = 5, frame = 0;
    int facingLeft = 0, walkingDown = 0, walkingUp = 0, moving = 0;
    Uint32 lastFrameTime = SDL_GetTicks();

    SDL_Event event;
    bool running = true;
    while (running)
    {
        while (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) running = false;

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        int prevX = playerX, prevY = playerY;
        moving = walkingDown = walkingUp = 0;
        int moveX = 0, moveY = 0;

        if (keys[SDL_SCANCODE_W]) {
            moveY -= speed;
            walkingUp = moving = 1;
        } else if (keys[SDL_SCANCODE_S]) {
            moveY += speed;
            walkingDown = moving = 1;
        }
        if (keys[SDL_SCANCODE_A]) {
            moveX -= speed;
            facingLeft = 1;
            moving = 1;
        } else if (keys[SDL_SCANCODE_D]) {
            moveX += speed;
            facingLeft = 0;
            moving = 1;
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
        if (moving && currentTime > lastFrameTime + FRAME_DELAY) {
            frame = (frame + 1) % FRAME_COUNT;
            lastFrameTime = currentTime;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderClear(renderer);

        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                SDL_Rect dst = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderCopy(renderer, tileTextures[map[y][x]], NULL, &dst);
            }
        }

        SDL_Rect treeRect = {300, 300, 128, 128};
        SDL_RenderCopy(renderer, treeTexture, NULL, &treeRect);

        SDL_Rect srcRect = { frame * 64, 0, 64, 90 };
        SDL_Rect destRect = { playerX, playerY, 128, 128 };

        if (moving) {
            if (walkingDown)      SDL_RenderCopy(renderer, walkDown, &srcRect, &destRect);
            else if (walkingUp)   SDL_RenderCopy(renderer, walkUp, &srcRect, &destRect);
            else if (facingLeft)  SDL_RenderCopy(renderer, walkLeft, &srcRect, &destRect);
            else                  SDL_RenderCopy(renderer, walkRight, &srcRect, &destRect);
        } else SDL_RenderCopy(renderer, idleFront, NULL, &destRect);


        SDL_RenderPresent(renderer);
        SDL_Delay(16);

    }

    SDL_DestroyTexture(treeTexture);
    SDL_DestroyTexture(walkRight);
    SDL_DestroyTexture(walkLeft);
    SDL_DestroyTexture(walkDown);
    SDL_DestroyTexture(walkUp);
    SDL_DestroyTexture(idleFront);
    for (int i = 0; i < 2; ++i) SDL_DestroyTexture(tileTextures[i]);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath){
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) { SDL_Log("Failed to load image: %s\n", IMG_GetError()); return NULL; }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

int selectCharacter(SDL_Renderer* renderer) {
    SDL_Surface* menuSurface = IMG_Load("lib/assets/meny.png");
    SDL_Texture* menuTexture = SDL_CreateTextureFromSurface(renderer, menuSurface);
    SDL_FreeSurface(menuSurface);

    SDL_Surface* grassSurface = IMG_Load("lib/assets/grass.png");
    SDL_Texture* grassTexture = SDL_CreateTextureFromSurface(renderer, grassSurface);
    SDL_FreeSurface(grassSurface);

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