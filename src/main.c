#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Cute Bunny", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface* frontSurf = IMG_Load("assets/bunny_front.png");
    SDL_Surface* backSurf = IMG_Load("assets/bunny_back.png");
    SDL_Surface* leftSurf = IMG_Load("assets/bunny_left.png");
    SDL_Surface* rightSurf = IMG_Load("assets/bunny_right.png");

    SDL_Surface* bgSurf = IMG_Load("assets/background.png");
    SDL_Texture* bgTex = SDL_CreateTextureFromSurface(renderer, bgSurf);
    SDL_FreeSurface(bgSurf);
    SDL_RenderCopy(renderer, bgTex, NULL, NULL);

    
    if (!frontSurf || !backSurf || !leftSurf || !rightSurf || !bgTex) {
        printf("Failed to load image: %s\n", IMG_GetError());
        return 1;
    }

    if (!bgTex) {
        printf("❌ Failed to load background.png: %s\n", IMG_GetError());
    }    

    SDL_Texture* frontTex = SDL_CreateTextureFromSurface(renderer, frontSurf);
    SDL_Texture* backTex = SDL_CreateTextureFromSurface(renderer, backSurf);
    SDL_Texture* leftTex = SDL_CreateTextureFromSurface(renderer, leftSurf);
    SDL_Texture* rightTex = SDL_CreateTextureFromSurface(renderer, rightSurf);

    int bunnyW = 50;
    int bunnyH = 70;

    int frontW = 50;
    int backW = 45;

    SDL_FreeSurface(frontSurf);
    SDL_FreeSurface(backSurf);
    SDL_FreeSurface(leftSurf);
    SDL_FreeSurface(rightSurf);

    SDL_Texture* currentTexture = frontTex;

    SDL_Rect dest = { 100, 300, bunnyW, bunnyH };
    int groundY = dest.y;

    int isJumping = 0;
    int velocityY = 0;
    int jumpVelocity = -15;
    int gravity = 1;

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE && !isJumping) {
                    isJumping = 1;
                    velocityY = jumpVelocity;
                }
            }
        }

        const Uint8* keystates = SDL_GetKeyboardState(NULL);

        if (!isJumping) {
            if (keystates[SDL_SCANCODE_UP]) {
                dest.y -= 5;
                currentTexture = backTex;
                groundY = dest.y;
            }
            if (keystates[SDL_SCANCODE_DOWN]) {
                dest.y += 5;
                currentTexture = frontTex;
                groundY = dest.y;
            }
        }

        if (keystates[SDL_SCANCODE_LEFT]) {
            dest.x -= 5;
            currentTexture = leftTex;
        }
        if (keystates[SDL_SCANCODE_RIGHT]) {
            dest.x += 5;
            currentTexture = rightTex;
        }

        if (isJumping) {
            dest.y += velocityY;
            velocityY += gravity;

            if (velocityY > 0 && dest.y >= groundY) {
                dest.y = groundY;
                isJumping = 0;
                velocityY = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 🖼️ Rita bakgrunden först
        SDL_RenderCopy(renderer, bgTex, NULL, NULL);

        // ✅ Sätt rätt storlek beroende på vilken textur det är
        if (currentTexture == frontTex) {
            dest.w = frontW;
            dest.h = bunnyH;
        } else if (currentTexture == backTex) {
            dest.w = backW;
            dest.h = bunnyH;
        } else {
            dest.w = bunnyW;
            dest.h = bunnyH;
        }

        SDL_RenderCopy(renderer, currentTexture, NULL, &dest);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(frontTex);
    SDL_DestroyTexture(backTex);
    SDL_DestroyTexture(leftTex);
    SDL_DestroyTexture(rightTex);
    SDL_DestroyTexture(bgTex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}