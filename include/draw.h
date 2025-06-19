#ifndef DRAW_H
#define DRAW_H

#include <string>
#include <SDL.h>

// --- Hàm vẽ chung ---
void prepareScene(void);
void presentScene(void);
void draw(void);

// --- Hàm vẽ các thành phần cụ thể (được công khai) ---
void drawBackground(void);
void drawStarfield(void);
void blit(SDL_Texture* texture, int x, int y);
void blitRect(SDL_Texture* texture, SDL_Rect* src, int x, int y);
void drawText(int x, int y, int r, int g, int b, int align, std::string outText);

#endif // DRAW_H