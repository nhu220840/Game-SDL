#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "init.h"
#include "globals.h"
#include "utils.h"
#include "stages.h"
#include "logic.h"
#include "draw.h"

static void initPlayer(void);
static void resetStage(void);
static void initStarfield(void);
static void initBackground(void);
static void initHighscoreTable(void);
static void loadSounds(void);
static void initFonts(void);
static void initSounds(void);


void initSDL(void) {
    int rendererFlags = SDL_RENDERER_ACCELERATED;
    int windowFlags = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    app.window = SDL_CreateWindow("Shooter Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);
    if (!app.window) {
        printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);
    if (!app.renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }

    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        printf("Couldn't initialize SDL Mixer: %s\n", Mix_GetError());
        exit(1);
    }
    Mix_AllocateChannels(MAX_SND_CHANNELS);
    SDL_ShowCursor(0);
}

void initGame(void) {
    initBackground();
    initStarfield();
    initSounds();
    initFonts();
    initHighscoreTable();
    loadMusic("assets/music/Mercury.ogg");
    playMusic(1);
}

void initStage(void) {
    app.delegate.logic = logic;
    app.delegate.draw = draw;
    memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);

    bulletTexture = loadTexture("assets/gfx/playerBullet.png");
    enemyTexture = loadTexture("assets/gfx/enemy.png");
    alienBulletTexture = loadTexture("assets/gfx/alienBullet.png");
    explosionTexture = loadTexture("assets/gfx/explosion.png");
    pointsTexture = loadTexture("assets/gfx/points.png");

    resetStage();
}

void initTitle(void) {
    app.delegate.logic = logic_Title;
    app.delegate.draw = draw_Title;
    memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);

    sdl2Texture = loadTexture("assets/gfx/sdl2.png");
    shooterTexture = loadTexture("assets/gfx/shooter.png");
    reveal = 0;
    timeout = FPS * 5;
}

void initHighscores(void) {
    app.delegate.logic = logic_HighSC;
    app.delegate.draw = draw_HighSC;
    memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
}

static void initPlayer(void) {
    player = (Entity*)malloc(sizeof(Entity));
    memset(player, 0, sizeof(Entity));
    stage.fighterTail->next = player;
    stage.fighterTail = player;
    player->health = 1;
    player->x = 100;
    player->y = 100;
    player->texture = loadTexture("assets/gfx/player.png");
    SDL_QueryTexture(player->texture, NULL, NULL, &player->w, &player->h);
    player->reload = 0;
    player->side = SIDE_PLAYER;
}

static void resetStage(void) {
    Entity* e;
    Explosion* ex;
    Debris* d;

    while (stage.fighterHead.next) {
        e = stage.fighterHead.next;
        stage.fighterHead.next = e->next;
        free(e);
    }
    while (stage.bulletHead.next) {
        e = stage.bulletHead.next;
        stage.bulletHead.next = e->next;
        free(e);
    }
    while (stage.explosionHead.next) {
        ex = stage.explosionHead.next;
        stage.explosionHead.next = ex->next;
        free(ex);
    }
    while (stage.debrisHead.next) {
        d = stage.debrisHead.next;
        stage.debrisHead.next = d->next;
        free(d);
    }
    while (stage.pointsHead.next) {
        e = stage.pointsHead.next;
        stage.pointsHead.next = e->next;
        free(e);
    }

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.bulletTail = &stage.bulletHead;
    stage.explosionTail = &stage.explosionHead;
    stage.debrisTail = &stage.debrisHead;
    stage.pointsTail = &stage.pointsHead;

    initPlayer();
    
    enemySpawnTimer = 0;
    stage.score = 0;
    stageResetTimer = FPS * 3;
}

static void initStarfield(void) {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x = rand() % SCREEN_WIDTH;
        stars[i].y = rand() % SCREEN_HEIGHT;
        stars[i].speed = 1 + rand() % 8;
    }
}

static void initBackground(void) {
    background = loadTexture("assets/gfx/background.png");
    backgroundX = 0;
}

static void initHighscoreTable(void) {
    memset(&highscores, 0, sizeof(Highscores));
    for (int i = 0; i < NUM_HIGHSCORES; i++) {
        highscores.highscore[i].score = 0;
        strncpy(highscores.highscore[i].name, "ANONYMOUS", MAX_SCORE_NAME_LENGTH - 1);
        highscores.highscore[i].name[MAX_SCORE_NAME_LENGTH - 1] = '\0';
    }
    newHighscoreFlag = false;
    cursorBlink = 0;
}

static void loadSounds(void) {
    sounds[SND_PLAYER_FIRE] = Mix_LoadWAV("assets/sound/334227__jradcoolness__laser.ogg");
    sounds[SND_ALIEN_FIRE] = Mix_LoadWAV("assets/sound/196914__dpoggioli__laser-gun.ogg");
    sounds[SND_PLAYER_DIE] = Mix_LoadWAV("assets/sound/245372__quaker540__hq-explosion.ogg");
    sounds[SND_ALIEN_DIE] = Mix_LoadWAV("assets/sound/10 Guage Shotgun-SoundBible.com-74120584.ogg");
    sounds[SND_POINTS] = Mix_LoadWAV("assets/sound/342749__rhodesmas__notification-01.ogg");
}

static void initSounds(void) { loadSounds(); }

static void initFonts(void) { fontTexture = loadTexture("assets/gfx/font.png"); }