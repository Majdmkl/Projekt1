#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "menu.h"


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
    
    initMenu(renderer);

    int selectedCharacter = -1;
    SDL_Event event;

    // Visa meny tills spelaren valt karaktär eller stängt spelet
    while (selectedCharacter == -1) {
        selectedCharacter = handleMenu(renderer, &event);
        if (selectedCharacter == -2) { // spelaren vill avsluta spelet helt
            cleanMenu();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            IMG_Quit();
            SDL_Quit();
            return 0;
        }
    }

    printf("Vald karaktär: %d\n", selectedCharacter); // för test

    SDL_Texture *characterFront, *characterBack, *characterLeft, *characterRight;

    if (selectedCharacter == 0) { // Bunny
        characterFront = IMG_LoadTexture(renderer, "assets/bunny_front.png");
        characterBack = IMG_LoadTexture(renderer, "assets/bunny_back.png");
        characterLeft = IMG_LoadTexture(renderer, "assets/bunny_left.png");
        characterRight = IMG_LoadTexture(renderer, "assets/bunny_right.png");
    }
    else if (selectedCharacter == 2) { // Panda
        characterFront = IMG_LoadTexture(renderer, "assets/panda_spritesheets/panda_left_foot.png");
        characterBack = IMG_LoadTexture(renderer, "assets/panda_spritesheets/panda_left_foot.png"); // Använd samma tills annan finns
        characterLeft = IMG_LoadTexture(renderer, "assets/panda_spritesheets/panda_walk_left.png");
        characterRight = IMG_LoadTexture(renderer, "assets/panda_spritesheets/panda_walk_right.png");
    }
    else {
        // Om användaren väljer en karaktär som inte är redo (t.ex. Lejon), avsluta programmet säkert:
        printf("Den valda karaktären finns ej än!\n");
        cleanMenu();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Kontrollera om bilderna laddades korrekt
    if (!characterFront || !characterBack || !characterLeft || !characterRight) {
        printf("Kunde inte ladda någon av karaktärsbilderna: %s\n", SDL_GetError());
        cleanMenu();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    

    SDL_Surface* tileSurfaces[2];
    tileSurfaces[0] = IMG_Load("assets/grass.png");
    tileSurfaces[1] = IMG_Load("assets/water.png");

    SDL_Texture* tileTextures[2];
    for (int i = 0; i < 2; ++i)
    {
        tileTextures[i] = SDL_CreateTextureFromSurface(renderer, tileSurfaces[i]);
        SDL_FreeSurface(tileSurfaces[i]);
    }

    SDL_Texture* bunnyWalkRight = IMG_LoadTexture(renderer, "assets/panda_spritesheets/panda_walk_right.png");
    SDL_Texture* bunnyWalkLeft = IMG_LoadTexture(renderer, "assets/panda_spritesheets/panda_walk_left.png");

    if (!bunnyWalkRight || !bunnyWalkLeft)
    {
        printf("Kunde inte ladda spritesheets: %s\n", SDL_GetError());
        return 1;
    }

    int frame = 0;
    Uint32 lastFrameTime = SDL_GetTicks();

    int playerX = 100;
    int playerY = 100;
    int speed = 10;
    int facingLeft = 0;

    int running = 1;
    while (running)
    {
        while (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) running = 0;

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
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            SDL_Rect dst = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
            SDL_RenderCopy(renderer, tileTextures[map[y][x]], NULL, &dst);
        }


        SDL_Rect destRect = { playerX, playerY, 64, 64 };

        if (moveY < 0) { // upp
            SDL_RenderCopy(renderer, characterBack, NULL, &destRect);
        } else if (moveY > 0) { // ner
            SDL_RenderCopy(renderer, characterFront, NULL, &destRect);
        } else if (moveX < 0) { // vänster
            SDL_RenderCopy(renderer, characterLeft, NULL, &destRect);
        } else if (moveX > 0) { // höger
            SDL_RenderCopy(renderer, characterRight, NULL, &destRect);
        } else {
            // Om stillastående, visa framåt-bild som default
            SDL_RenderCopy(renderer, characterFront, NULL, &destRect);
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    for (int i = 0; i < 2; ++i) SDL_DestroyTexture(tileTextures[i]);


    cleanMenu(); // frigör meny-resurser
    SDL_DestroyTexture(characterFront);
    SDL_DestroyTexture(characterBack);
    SDL_DestroyTexture(characterLeft);
    SDL_DestroyTexture(characterRight);    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}