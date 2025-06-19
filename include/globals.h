#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL_mixer.h>
#include "structs.h"

// -- Khai báo các biến toàn cục --

extern App app;
extern Stage stage;
extern Highscores highscores;
extern Highscore newHighscore;
extern Entity* player;
extern Star stars[MAX_STARS + 1];

extern SDL_Texture* bulletTexture;
extern SDL_Texture* enemyTexture;
extern SDL_Texture* alienBulletTexture;
extern SDL_Texture* explosionTexture;
extern SDL_Texture* fontTexture;
extern SDL_Texture* pointsTexture;
extern SDL_Texture* sdl2Texture;
extern SDL_Texture* shooterTexture;
extern SDL_Texture* background;

extern Mix_Music* music;
extern Mix_Chunk* sounds[MAX_LINE_LENGTH];

extern int enemySpawnTimer;
extern int stageResetTimer;
extern int backgroundX;
extern int highscore;
extern int cursorBlink;
extern bool newHighscoreFlag;
extern int timeout;
extern int reveal;

#endif // GLOBALS_H