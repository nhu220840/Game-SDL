#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

#include "stages.h"
#include "globals.h"
#include "draw.h"
#include "init.h"
#include "logic.h"
#include "utils.h"

// Khai báo các hàm tĩnh chỉ dùng trong file này
static void drawLogo(void);
static void drawHighscores(void);
static void doNameInput(void);
static void drawNameInput(void);
static void doBackground(void);
static void doStarfield(void);

// Hàm được lấy từ logic.cpp để dùng chung
static void doBackground(void) { if (--backgroundX < -SCREEN_WIDTH) backgroundX = 0; }
static void doStarfield(void) {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x -= stars[i].speed;
        if (stars[i].x < 0) stars[i].x = SCREEN_WIDTH + stars[i].x;
    }
}

// --- LOGIC VÀ DRAW CHO MÀN HÌNH TITLE ---

void logic_Title(void) {
    doBackground();
    doStarfield();
    if (reveal < SCREEN_HEIGHT) reveal++;
    if (--timeout <= 0) initHighscores();
    if (app.keyboard[SDL_SCANCODE_LCTRL]) initStage();
}

void draw_Title(void) {
    // SỬA LỖI: Chỉ vẽ những gì cần thiết, không gọi hàm draw() chính
    drawBackground();
    drawStarfield();
    drawLogo();
    if (timeout % 40 < 20) {
        drawText(SCREEN_WIDTH / 2, 600, 255, 255, 255, TEXT_CENTER, "PRESS FIRE TO PLAY!");
    }
}

static void drawLogo(void) {
    SDL_Rect r;
    r.x = 0;
    r.y = 0;

    SDL_QueryTexture(sdl2Texture, NULL, NULL, &r.w, &r.h);
    r.h = fmin(reveal, r.h);
    blitRect(sdl2Texture, &r, (SCREEN_WIDTH / 2) - (r.w / 2), 100);

    SDL_QueryTexture(shooterTexture, NULL, NULL, &r.w, &r.h);
    r.h = fmin(reveal, r.h);
    blitRect(shooterTexture, &r, (SCREEN_WIDTH / 2) - (r.w / 2), 250);
}


// --- LOGIC VÀ DRAW CHO MÀN HÌNH HIGHSCORE ---

void logic_HighSC(void) {
    doBackground();
    doStarfield();
    if (newHighscoreFlag) doNameInput();
    else if (app.keyboard[SDL_SCANCODE_LCTRL]) initStage();

    if (++cursorBlink >= FPS) cursorBlink = 0;
}

void draw_HighSC(void) {
    // SỬA LỖI: Chỉ vẽ những gì cần thiết, không gọi hàm draw() chính
    drawBackground();
    drawStarfield();
    if (newHighscoreFlag) drawNameInput();
    else drawHighscores();
}

static void drawHighscores(void) {
    char text[128];
    drawText(SCREEN_WIDTH / 2, 70, 255, 255, 255, TEXT_CENTER, "HIGH SCORES");
    int y = 150;
    for (int i = 0; i < NUM_HIGHSCORES; i++) {
        sprintf(text, "#%d %-15s %05d", i + 1, highscores.highscore[i].name, highscores.highscore[i].score);
        if (highscores.highscore[i].recent) {
            drawText(SCREEN_WIDTH / 2, y, 255, 255, 0, TEXT_CENTER, text);
        } else {
            drawText(SCREEN_WIDTH / 2, y, 255, 255, 255, TEXT_CENTER, text);
        }
        y += 50;
    }
    drawText(SCREEN_WIDTH / 2, y + 50, 255, 255, 255, TEXT_CENTER, "PRESS FIRE TO PLAY!");
}

static void doNameInput(void) {
    size_t n = strlen(newHighscore.name);
    if (strlen(app.inputText) > 0) {
        for (int i = 0; app.inputText[i] != '\0'; i++) {
            char c = toupper(app.inputText[i]);
            if (n < MAX_SCORE_NAME_LENGTH - 1 && c >= ' ' && c <= 'Z') {
                newHighscore.name[n++] = c;
            }
        }
        newHighscore.name[n] = '\0';
    }

    if (n > 0 && app.keyboard[SDL_SCANCODE_BACKSPACE]) {
        newHighscore.name[n - 1] = '\0';
        app.keyboard[SDL_SCANCODE_BACKSPACE] = 0;
    }

    if (app.keyboard[SDL_SCANCODE_RETURN]) {
        if (strlen(newHighscore.name) == 0) {
            strncpy(newHighscore.name, "ANONYMOUS", MAX_SCORE_NAME_LENGTH - 1);
        }
        for (int i = 0; i < NUM_HIGHSCORES; ++i) {
            if (highscores.highscore[i].recent) {
                strncpy(highscores.highscore[i].name, newHighscore.name, MAX_SCORE_NAME_LENGTH - 1);
            }
        }
        newHighscoreFlag = false;
        newHighscore.name[0] = '\0';
        app.keyboard[SDL_SCANCODE_RETURN] = 0;
    }
}

static void drawNameInput(void) {
    SDL_Rect r;
    drawText(SCREEN_WIDTH / 2, 70, 255, 255, 255, TEXT_CENTER, "CONGRATULATIONS, YOU'VE GAINED A HIGHSCORE!");
    drawText(SCREEN_WIDTH / 2, 120, 255, 255, 255, TEXT_CENTER, "ENTER YOUR NAME BELOW:");
    drawText(SCREEN_WIDTH / 2, 250, 128, 255, 128, TEXT_CENTER, newHighscore.name);
    
    if (cursorBlink < FPS / 2) {
        r.x = (SCREEN_WIDTH / 2) + (strlen(newHighscore.name) * GLYPH_WIDTH) / 2 + 5;
        r.y = 250;
        r.w = GLYPH_WIDTH;
        r.h = GLYPH_HEIGHT;
        SDL_SetRenderDrawColor(app.renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(app.renderer, &r);
    }
    
    drawText(SCREEN_WIDTH / 2, 625, 255, 255, 255, TEXT_CENTER, "PRESS RETURN WHEN FINISHED");
}