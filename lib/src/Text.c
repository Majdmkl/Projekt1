#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "Text.h"

typedef struct  text {
  SDL_Rect rect;
  SDL_Texture *texture;
  SDL_Renderer *renderer;
} Text;

Text *createText(SDL_Renderer *renderer, int r, int g, int b, TTF_Font *font, char *string, int x, int y) {
    if (!renderer || !font || !string) return NULL;

    Text *text = (Text *)malloc(sizeof(Text));
    if (!text) return NULL;

    text->renderer = renderer;

    SDL_Color color = {r, g, b, 255};
    TTF_SetFontOutline(font, 1);
    SDL_Surface *surface = TTF_RenderText_Blended(font, string, color);
    TTF_SetFontOutline(font, 0);
    if (!surface) { free(text); return NULL; }

    text->texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!text->texture) {
        SDL_FreeSurface(surface);
        free(text);
        return NULL;
    }

    text->rect.x = x;
    text->rect.y = y;
    text->rect.w = surface->w;
    text->rect.h = surface->h;

    SDL_FreeSurface(surface);

    return text;
}

void drawText(Text *text) { SDL_RenderCopy(text->renderer, text->texture, NULL, &text->rect); }

void destroyText(Text *text) {
    SDL_DestroyTexture(text->texture);
    free(text);
}

void drawTextCentered(Text *text, int centerX, int centerY) {
    text->rect.x = centerX - text->rect.w / 2;
    text->rect.y = centerY - text->rect.h / 2;
    SDL_RenderCopy(text->renderer, text->texture, NULL, &text->rect);
}