#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Character.h"
#include "Map.h"


void initSDL();
SDL_Window* createWindow();
int selectCharacter(SDL_Renderer* renderer);
SDL_Renderer* createRenderer(SDL_Window* window);
void gameLoop(SDL_Renderer* renderer, Character* player);
void cleanup(SDL_Window* window, SDL_Renderer* renderer);
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath);

int main(int argc, char* argv[]) {
    initSDL();

    SDL_Window* window = createWindow();
    SDL_Renderer* renderer = createRenderer(window);

    int selected = selectCharacter(renderer);
    if (selected == -1) {
        cleanup(window, renderer);
        return 1;
    }

    Character* player = createCharacter(renderer, selected);
    if (!player) {
        SDL_Log("Could not create character.");
        cleanup(window, renderer);
        return 1;
    }

    gameLoop(renderer, player);

    destroyCharacter(player);
    cleanup(window, renderer);

    return 0;
}

void initSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
}

SDL_Window* createWindow() {
    return SDL_CreateWindow("COZY DELIVERY", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

SDL_Renderer* createRenderer(SDL_Window* window) {
    return SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) {
        SDL_Log("Failed to load image: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void gameLoop(SDL_Renderer* renderer, Character* player) {
    MAP* gameMap = createMap(renderer);
    if (!gameMap) {
        SDL_Log("Failed to create map");
        return;
    }

    #define MAX_BULLETS 100
    Bullet* bullets[MAX_BULLETS];
    int bulletCount = 0;

    SDL_Event event;
    bool running = true;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                float startX = player->x + CHARACTER_WIDTH / 2.0f;
                float startY = player->y + CHARACTER_HEIGHT / 2.0f;

                if (bulletCount < MAX_BULLETS) {
                    bullets[bulletCount++] = createBullet(renderer, startX, startY, mouseX - startX, mouseY - startY, 0);
                }
            }
        }
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                float startX = player->x + CHARACTER_WIDTH / 2.0f;
                float startY = player->y + CHARACTER_HEIGHT / 2.0f;

                if (bulletCount < MAX_BULLETS) {
                    bullets[bulletCount++] = createBullet(renderer, startX, startY, mouseX - startX, mouseY - startY, 0);
                }
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        float moveX = 0, moveY = 0;

        if (keys[SDL_SCANCODE_W]) {
            moveY -= MOVE_SPEED;
            turnUp(player);
        }
        if (keys[SDL_SCANCODE_S]) {
            moveY += MOVE_SPEED;
            turnDown(player);
        }
        if (keys[SDL_SCANCODE_A]) {
            moveX -= MOVE_SPEED;
            turnLeft(player);
        }
        if (keys[SDL_SCANCODE_D]) {
            moveX += MOVE_SPEED;
            turnRight(player);
        }

        if (moveX != 0 && moveY != 0) {
            float diagSpeed = MOVE_SPEED / 1.4142f;
            moveX = (moveX > 0) ? diagSpeed : -diagSpeed;
            moveY = (moveY > 0) ? diagSpeed : -diagSpeed;
        }

        moveCharacter(player, moveX, moveY, walls, 23);

        updateCharacterAnimation(player, SDL_GetTicks());

        Uint32 now = SDL_GetTicks();
        for (int i = 0; i < bulletCount; ) {
            Bullet* b = bullets[i];
            if ((now - b->bornTime > BULLETLIFETIME) || checkCollisionBulletWall(b, walls, 23)) {
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

        for (int i = 0; i < bulletCount; i++) {
            drawBullet(bullets[i], renderer);
        }

        healthBar(player, renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 fps
    }

    for (int i = 0; i < bulletCount; i++) {
        destroyBullet(bullets[i]);
    }
    destroyMap(gameMap);
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

void cleanup(SDL_Window* window, SDL_Renderer* renderer) {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();
}