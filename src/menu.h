// menu.h
#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Initialize menu resources (textures etc)
void initMenu(SDL_Renderer *renderer);

// Handle menu input and render menu screen
// Return selected character index if selected, -1 otherwise
int handleMenu(SDL_Renderer *renderer, SDL_Event *event);

// Cleanup menu resources
void cleanMenu(void);

#endif //MENU_H