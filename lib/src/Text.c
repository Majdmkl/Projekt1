#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    SDL_Rect rect;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
} Text;

Text *createText(SDL_Renderer *renderer, int r, int g, int b, TTF_Font *font, const char *string, int x, int y) {
    if (!renderer || !font || !string) return NULL;

    SDL_Color color = {r, g, b, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, string, color);
    if (!surface) return NULL;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return NULL;
    }

    Text *text = malloc(sizeof(Text));
    if (!text) {
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        return NULL;
    }

    text->renderer = renderer;
    text->texture = texture;
    text->rect = (SDL_Rect){ x, y, surface->w, surface->h };

    SDL_FreeSurface(surface);
    return text;
}

void drawText(Text *text) {
    if (text && text->texture && text->renderer) SDL_RenderCopy(text->renderer, text->texture, NULL, &text->rect);
}

void destroyText(Text *text) {
    if (text) {
        SDL_DestroyTexture(text->texture);
        free(text);
    }
}