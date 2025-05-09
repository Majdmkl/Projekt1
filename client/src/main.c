#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "Character.h"
#include "Map.h"


void initSDL();
SDL_Window* createWindow();
int selectCharacter(SDL_Renderer* renderer);
SDL_Renderer* createRenderer(SDL_Window* window);
void gameLoop(SDL_Renderer* renderer, Character* player);
void cleanup(SDL_Window* window, SDL_Renderer* renderer);
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath);
int mainMenu(SDL_Renderer* renderer);
char* connectionScreen(SDL_Renderer* renderer);
void waitingRoom(SDL_Renderer* renderer);

int main(int argc, char* argv[]) {
    initSDL();

    SDL_Window* window = createWindow();
    SDL_Renderer* renderer = createRenderer(window);

    // 👉 Först visa huvudmenyn
    int menuSelection = mainMenu(renderer);
    if (menuSelection == 1) {
        char* ip = connectionScreen(renderer);
        if (ip) {
            int selected = selectCharacter(renderer);
            if (selected == -1) {
                cleanup(window, renderer);
                return 1;
            }
    
            waitingRoom(renderer); // ny funktion
    
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
        cleanup(window, renderer);
        return 0;
    }      
    
    if (menuSelection == 2) {
        cleanup(window, renderer);
        return 0;
    }    

    // 👉 Visa karaktärsval
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

    // 👉 Starta spelet
    gameLoop(renderer, player);

    destroyCharacter(player);
    cleanup(window, renderer);
    return 0;
}

void initSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
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

        bool isMoving = false;

        if (keys[SDL_SCANCODE_W]) {
            moveY -= MOVE_SPEED;
            turnUp(player);
            isMoving = true;
        }
        if (keys[SDL_SCANCODE_S]) {
            moveY += MOVE_SPEED;
            turnDown(player);
            isMoving = true;
        }
        if (keys[SDL_SCANCODE_A]) {
            moveX -= MOVE_SPEED;
            turnLeft(player);
            isMoving = true;
        }
        if (keys[SDL_SCANCODE_D]) {
            moveX += MOVE_SPEED;
            turnRight(player);
            isMoving = true;
        }

        // Om ingen rörelse skedde → ställ spelaren till IDLE
        if (!isMoving) {
            player->state = IDLE;
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

char* connectionScreen(SDL_Renderer* renderer) {
    static char ip[64] = "";
    bool typingActive = false;

    SDL_Texture* menuTexture = loadTexture(renderer, "lib/assets/startPage/ip.png");
    SDL_Texture* grassTexture = loadTexture(renderer, "lib/assets/grass.png");

    TTF_Font* font = TTF_OpenFont("lib/assets/Press_Start_2P/PressStart2P-Regular.ttf", 16);
    if (!font) {
        SDL_Log("Kunde inte ladda font: %s", TTF_GetError());
        return NULL;
    }

    SDL_Rect menuRect = {
        (SCREEN_WIDTH - 800) / 2,
        (SCREEN_HEIGHT - 900) / 2,
        800, 900
    };

    SDL_Rect inputBox = {
        menuRect.x + 240,
        menuRect.y + 450,
        340, 70
    };

    SDL_Rect closeBtn = {
        inputBox.x + inputBox.w - 45,
        inputBox.y + 15,
        40, 40
    };

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
                } else {
                    typingActive = false;
                    SDL_StopTextInput();
                }

                if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &closeBtn)) {
                    ip[0] = '\0';
                    typingActive = false;
                    SDL_StopTextInput();
                }
            }

            else if (typingActive && event.type == SDL_TEXTINPUT) {
                strncat(ip, event.text.text, sizeof(ip) - strlen(ip) - 1);
            }

            else if (typingActive && event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(ip) > 0) {
                    ip[strlen(ip) - 1] = '\0';
                }
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
            SDL_Rect outline = {
                menuRect.x - i,
                menuRect.y - i,
                menuRect.w + 2 * i,
                menuRect.h + 2 * i
            };
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

            SDL_Rect textRect = {
                inputBox.x + 10,
                inputBox.y + (inputBox.h - textSurface->h) / 2,
                textSurface->w,
                textSurface->h
            };

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
    SDL_Texture* bgTexture = loadTexture(renderer, "lib/assets/startPage/waitingRoom.png");
    SDL_Texture* grassTexture = loadTexture(renderer, "lib/assets/grass.png");

    SDL_Event event;

    SDL_Rect menuRect = {
        (SCREEN_WIDTH - 700) / 2,
        (SCREEN_HEIGHT - 900) / 2,
        690, 910
    };

    SDL_Rect continueBtn = {
        menuRect.x + 170,
        menuRect.y + 765,
        390, 85
    };

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
            SDL_Rect outline = {
                menuRect.x - i,
                menuRect.y - i,
                menuRect.w + 2 * i,
                menuRect.h + 2 * i
            };
            SDL_RenderDrawRect(renderer, &outline);
        }

        // Vit ruta runt continue-knapp (för debug)
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // SDL_RenderDrawRect(renderer, &continueBtn);

        SDL_RenderPresent(renderer);
    }
}


int mainMenu(SDL_Renderer* renderer) {
    SDL_Texture* menuTexture = loadTexture(renderer, "lib/assets/startPage/startMenyn.png");
    SDL_Texture* grassTexture = loadTexture(renderer, "lib/assets/grass.png");

    SDL_Event event;
    int selection = -1;

    SDL_Rect menuRect = {
        (SCREEN_WIDTH - 700) / 2,
        (SCREEN_HEIGHT - 900) / 2,
        700, 900
    };

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
            SDL_Rect outline = {
                menuRect.x - i,
                menuRect.y - i,
                menuRect.w + 2 * i,
                menuRect.h + 2 * i
            };
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
    SDL_Texture* menuTexture = loadTexture(renderer, "lib/assets/startPage/selChar.png");
    SDL_Texture* grassTexture = loadTexture(renderer, "lib/assets/grass.png");

    SDL_Event event;
    int selected = -1;

    SDL_Rect menuRect = {
        (SCREEN_WIDTH - 700) / 2,
        (SCREEN_HEIGHT - 900) / 2,
        700, 900
    };

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
            SDL_Rect outline = {
                menuRect.x - i,
                menuRect.y - i,
                menuRect.w + 2 * i,
                menuRect.h + 2 * i
            };
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

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}