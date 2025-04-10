#include "menu.h"
#include <stdio.h>

#define TOTAL_CHARACTERS 3

SDL_Texture* characters[TOTAL_CHARACTERS];
int currentCharacter = 1; // Börja med lejon (index 1)

// Ladda texturer för karaktärer
void initMenu(SDL_Renderer *renderer) {
    // ERSÄTT MED RIKTIGA BILDER OM DE FINNS
    // Exempelbilder som placeholders
    characters[0] = IMG_LoadTexture(renderer, "assets/bunny_front.png");  // BUNNY
    characters[1] = IMG_LoadTexture(renderer, "assets/image.png");  // LEJON
    characters[2] = IMG_LoadTexture(renderer, "assets/panda_spritesheets/panda_left_foot.png"); // PANDA

    for (int i = 0; i < TOTAL_CHARACTERS; i++) {
        if (!characters[i]) {
            printf("VARNING: Kunde inte ladda karaktär %d! Lägg till rätt bildfil!\n", i);
        }
    }
}

// Hantera input och rendera menyn
int handleMenu(SDL_Renderer *renderer, SDL_Event *event) {
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT)
            return -2; // Specialkod för att avsluta spelet helt.

        if (event->type == SDL_KEYDOWN) {
            if (event->key.keysym.sym == SDLK_a) {
                currentCharacter--;
                if (currentCharacter < 0) currentCharacter = TOTAL_CHARACTERS - 1;
            }
            if (event->key.keysym.sym == SDLK_d) {
                currentCharacter++;
                if (currentCharacter >= TOTAL_CHARACTERS) currentCharacter = 0;
            }
            if (event->key.keysym.sym == SDLK_SPACE) {
                return currentCharacter; // Vald karaktär.
            }
        }
    }

    // Rendera gråtonad bakgrund
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 150);
    SDL_RenderFillRect(renderer, NULL);

    // Rendera vald karaktär i mitten
    SDL_Rect dest = {640 - 64, 360 - 64, 128, 128}; // centrerad 128x128
    if (characters[currentCharacter]) {
        SDL_RenderCopy(renderer, characters[currentCharacter], NULL, &dest);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);

    return -1; // Ingen karaktär vald ännu.
}

// Frigör resurser
void cleanMenu(void) {
    for (int i = 0; i < TOTAL_CHARACTERS; i++) {
        if (characters[i])
            SDL_DestroyTexture(characters[i]);
    }
}
