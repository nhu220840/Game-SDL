#include <string>
#include <algorithm>
#include <stdio.h>

#include "draw.h"
#include "globals.h"
#include "utils.h"


static void drawFighters(void);
static void drawBullets(void);
static void drawDebris(void);
static void drawExplosions(void);
static void drawHud(void);
static void drawPointsPods(void);

void blit(SDL_Texture* texture, int x, int y) {
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
    SDL_RenderCopy(app.renderer, texture, NULL, &dest);
}

void blitRect(SDL_Texture* texture, SDL_Rect* src, int x, int y) {
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    dest.w = src->w;
    dest.h = src->h;
    SDL_RenderCopy(app.renderer, texture, src, &dest);
}

void prepareScene(void) {
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
    SDL_RenderClear(app.renderer);
}

void presentScene(void) {
    SDL_RenderPresent(app.renderer);
}

void draw(void) {
    drawBackground();
    drawStarfield();
    drawPointsPods();
    drawFighters();
    drawBullets();
    drawDebris();
    drawExplosions();
    drawHud();
}

void drawBackground(void) {
    SDL_Rect dest;
    for (int x = backgroundX; x < SCREEN_WIDTH; x += SCREEN_WIDTH) {
        dest.x = x;
        dest.y = 0;
        dest.w = SCREEN_WIDTH;
        dest.h = SCREEN_HEIGHT;
        SDL_RenderCopy(app.renderer, background, NULL, &dest);
    }
}

void drawStarfield(void) {
    int c;
    for (int i = 0; i < MAX_STARS; i++) {
        c = 32 * stars[i].speed;
        SDL_SetRenderDrawColor(app.renderer, c, c, c, 255);
        SDL_RenderDrawLine(app.renderer, stars[i].x, stars[i].y, stars[i].x + 3, stars[i].y);
    }
}

static void drawFighters(void) {
    for (Entity* e = stage.fighterHead.next; e != NULL; e = e->next) {
        blit(e->texture, e->x, e->y);
    }
}

static void drawBullets(void) {
    for (Entity* b = stage.bulletHead.next; b != NULL; b = b->next) {
        blit(b->texture, b->x, b->y);
    }
}

static void drawDebris(void) {
    for (Debris* d = stage.debrisHead.next; d != NULL; d = d->next) {
        blitRect(d->texture, &d->rect, d->x, d->y);
    }
}

static void drawExplosions(void) {
    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_ADD);
    SDL_SetTextureBlendMode(explosionTexture, SDL_BLENDMODE_ADD);

    for (Explosion* e = stage.explosionHead.next; e != NULL; e = e->next) {
        SDL_SetTextureColorMod(explosionTexture, e->r, e->g, e->b);
        SDL_SetTextureAlphaMod(explosionTexture, e->a);
        blit(explosionTexture, e->x, e->y);
    }

    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
}

void drawText(int x, int y, int r, int g, int b, int align, std::string outText) {
    int len = outText.length();
    SDL_Rect rect;
    rect.w = GLYPH_WIDTH;
    rect.h = GLYPH_HEIGHT;
    rect.y = 0;

    switch (align) {
        case TEXT_RIGHT: x -= (len * GLYPH_WIDTH); break;
        case TEXT_CENTER: x -= (len * GLYPH_WIDTH) / 2; break;
    }

    std::transform(outText.begin(), outText.end(), outText.begin(), ::toupper);
    SDL_SetTextureColorMod(fontTexture, r, g, b);

    for (int i = 0; i < len; i++) {
        if (outText[i] >= ' ' && outText[i] <= 'Z') {
            rect.x = (outText[i] - ' ') * GLYPH_WIDTH;
            blitRect(fontTexture, &rect, x, y);
            x += GLYPH_WIDTH;
        }
    }
}

static void drawHud(void) {
    std::string scoreText = "SCORE: " + numberfill(stage.score);
    drawText(20, 10, 255, 255, 255, TEXT_LEFT, scoreText);

    std::string highscoreText = "HIGH SCORE: " + numberfill(highscore);
    if (stage.score > 0 && stage.score == highscore) {
        drawText(SCREEN_WIDTH - 20, 10, 0, 255, 0, TEXT_RIGHT, highscoreText);
    } else {
        drawText(SCREEN_WIDTH - 20, 10, 255, 255, 255, TEXT_RIGHT, highscoreText);
    }
}

static void drawPointsPods(void) {
    for (Entity* e = stage.pointsHead.next; e != NULL; e = e->next) {
        if (e->health > (FPS * 2) || (e->health % 12) < 6) {
            blit(e->texture, e->x, e->y);
        }
    }
}