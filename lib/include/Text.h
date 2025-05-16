#ifndef Text_h
#define Text_h

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct text Text;

void drawText(Text *pText);
void drawTextCentered(Text *pText, int centerX, int centerY);
void destroyText(Text *pText);
Text *createText(SDL_Renderer *pRenderer, int r, int g, int b, TTF_Font *pFont, char *pString, int x, int y);

#endif