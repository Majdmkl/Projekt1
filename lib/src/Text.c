#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    SDL_Rect rect;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
} Text;

Text *createText(SDL_Renderer *renderer, TTF_Font *font, const char *string, SDL_Color color, int x, int y) {
    if (!renderer || !font || !string) return NULL;

    SDL_Surface *surface = TTF_RenderText_Solid(font, string, color);
    if (!surface) return NULL;

    Text *text = malloc(sizeof(Text));
    if (!text) { SDL_FreeSurface(surface); return NULL;}

    text->texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!text->texture) {
        free(text);
        SDL_FreeSurface(surface);
        return NULL;
    }

    text->rect = (SDL_Rect){ x, y, surface->w, surface->h };
    SDL_FreeSurface(surface);
    return text;
}

void drawText(Text *text) { SDL_RenderCopy(text->renderer, text->texture, NULL, &text->rect); }
void destroyText(Text *text) { SDL_DestroyTexture(text->texture); free(text); }