#ifndef text_h
#define text_h

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct text Text;

Text *createText(SDL_Renderer *pRenderer, int r, int g, int b, TTF_Font *pFont, char *pString, int x, int y);
void drawText(Text *pText);
void destroyText(Text *pText);

#endif