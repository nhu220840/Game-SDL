#include <SDL.h>
#include <stdlib.h>
#include <time.h>

#include "init.h"
#include "input.h"
#include "utils.h"
#include "globals.h"
#include "draw.h"

// --- ĐỊNH NGHĨA BIẾN TOÀN CỤC ---
// Các biến này được khai báo 'extern' trong globals.h
// và được định nghĩa (cấp phát bộ nhớ) duy nhất ở đây.
App app;
Stage stage;
Highscores highscores;
Highscore newHighscore;
Entity* player;
Star stars[MAX_STARS + 1];
SDL_Texture* bulletTexture;
SDL_Texture* enemyTexture;
SDL_Texture* alienBulletTexture;
SDL_Texture* explosionTexture;
SDL_Texture* fontTexture;
SDL_Texture* pointsTexture;
SDL_Texture* sdl2Texture;
SDL_Texture* shooterTexture;
SDL_Texture* background;
Mix_Music* music;
Mix_Chunk* sounds[MAX_LINE_LENGTH];
int enemySpawnTimer;
int stageResetTimer;
int backgroundX;
int highscore;
int cursorBlink;
bool newHighscoreFlag;
int timeout;
int reveal;


void cleanup(void) {
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    long then;
    float remainder;

    memset(&app, 0, sizeof(App));
    app.textureTail = &app.textureHead;
    srand(time(NULL));

    initSDL();
    atexit(cleanup);

    initGame();
    initTitle();

    then = SDL_GetTicks();
    remainder = 0;
    highscore = 0;

    SDL_StartTextInput();

    while (1) {
        prepareScene();
        doInput();
        app.delegate.logic();
        app.delegate.draw();
        presentScene();
        capFrameRate(&then, &remainder);
    }

    return 0;
}