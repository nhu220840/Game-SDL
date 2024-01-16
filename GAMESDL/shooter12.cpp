#include <iostream>
#include <stdio.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <string>
#include <algorithm>


#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define PLAYER_SPEED 4
#define PLAYER_BULLET_SPEED 16
#define ALIEN_BULLET_SPEED 8

#define MAX_KEYBOARD_KEYS 350
#define MAX_STARS 500

#define SIDE_PLAYER 0
#define SIDE_ALIEN 1

#define FPS 60

#define MAX_SND_CHANNELS 8
#define MAX_LINE_LENGTH 1024

#define GLYPH_WIDTH 18
#define GLYPH_HEIGHT 28

enum {
    CH_ANY = -1,
    CH_PLAYER,
    CH_ALIEN_FIRE,
    CH_POINTS
};

enum {
    SND_PLAYER_FIRE,
    SND_ALIEN_FIRE,
    SND_PLAYER_DIE,
    SND_ALIEN_DIE,
    SND_MAX,
    SND_POINTS
};

struct Delegate {
    void (*logic)(void);
    void (*draw)(void);
};

struct App {
    SDL_Renderer* renderer;
    SDL_Window* window;
    Delegate delegate;
    int keyboard[MAX_KEYBOARD_KEYS];
};

struct Entity {
    float x;
    float y;
    int w;
    int h;
    float dx;
    float dy;
    int health;
    int reload;
    int side;
    SDL_Texture* texture;
    struct Entity* next;
};

struct Explosion {
    float x;
    float y;
    float dx;
    float dy;
    int r, g, b, a;
    Explosion* next;
};

struct Debris {
    float x;
    float y;
    float dx;
    float dy;
    SDL_Rect rect;
    SDL_Texture* texture;
    int life;
    Debris* next;
};

struct Stage {
    Entity fighterHead, * fighterTail;
    Entity bulletHead, * bulletTail;
    Entity pointsHead, * pointsTail;
    Explosion explosionHead, * explosionTail;
    Debris debrisHead, * debrisTail;
    int score;

};

struct Star {
    int x;
    int y;
    int speed;
};

App app;
Entity* player;
Entity* bullet;
Stage stage;
Star stars[MAX_STARS + 1];
SDL_Texture* bulletTexture;
SDL_Texture* enemyTexture;
SDL_Texture* alienBulletTexture;
SDL_Texture* playerTexture;
SDL_Texture* background;
SDL_Texture* explosionTexture;
SDL_Texture* fontTexture;
SDL_Texture* pointsTexture;
Mix_Music* music;
Mix_Chunk* sounds[MAX_LINE_LENGTH];
int enemySpawnTimer;
int stageResetTimer;
int backgroundX;
int highscore;

int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (fmax(x1, x2) < fmin(x1 + w1, x2 + w2)) && (fmax(y1, y2) < fmin(y1 + h1, y2 + h2));
}

void calcSlope(int x1, int y1, int x2, int y2, float* dx, float* dy) {
    int steps = fmax(abs(x1 - x2), abs(y1 - y2));

    if (steps == 0) {
        *dx = *dy = 0;
        return;
    }

    *dx = x1 - x2;
    *dx /= steps;

    *dy = y1 - y2;
    *dy /= steps;
}

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

SDL_Texture* loadTexture(const char* filename) {
    SDL_Texture* texture;

    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

    texture = IMG_LoadTexture(app.renderer, filename);

    return texture;
}

void prepareScene(void) {
    SDL_SetRenderDrawColor(app.renderer, 96, 128, 255, 255);
    SDL_RenderClear(app.renderer);
}

void presentScene(void) {
    SDL_RenderPresent(app.renderer);
}


static void loadSounds(void) {
    sounds[SND_PLAYER_FIRE] = Mix_LoadWAV("sound/334227__jradcoolness__laser.ogg");
    sounds[SND_ALIEN_FIRE] = Mix_LoadWAV("sound/196914__dpoggioli__laser-gun.ogg");
    sounds[SND_PLAYER_DIE] = Mix_LoadWAV("sound/245372__quaker540__hq-explosion.ogg");
    sounds[SND_ALIEN_DIE] = Mix_LoadWAV("sound/10 Guage Shotgun-SoundBible.com-74120584.ogg");
    sounds[SND_POINTS] = Mix_LoadWAV("sound/342749__rhodesmas__notification-01.ogg");
}

void initSounds(void) {

    loadSounds();
}

void playSound(int id, int channel) {
    Mix_PlayChannel(channel, sounds[id], 0);
}


void loadMusic(char* filename) {
    if (music != NULL) {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
    }

    music = Mix_LoadMUS(filename);
}

void playMusic(int loop) {
    Mix_PlayMusic(music, (loop) ? -1 : 0);
}

void initFonts(void) {
    fontTexture = loadTexture("gfx/font.png");
}

void initSDL(void) {
    int rendererFlags, windowFlags;

    rendererFlags = SDL_RENDERER_ACCELERATED;

    windowFlags = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Couldn't initialize SDL: %d\n", SDL_GetError());
        exit(1);
    }

    app.window = SDL_CreateWindow("Shooter 11", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);
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

    SDL_ShowCursor(0);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        printf("Couldn't initialize SDL Mixer\n");
        exit(1);
    }

    Mix_AllocateChannels(MAX_SND_CHANNELS);
}

void doKeyDown(SDL_KeyboardEvent* event) {
    if (event->repeat == 0 && event->keysym.scancode < MAX_KEYBOARD_KEYS) {
        app.keyboard[event->keysym.scancode] = 1;
    }
}

void doKeyUp(SDL_KeyboardEvent* event) {
    if (event->repeat == 0 && event->keysym.scancode < MAX_KEYBOARD_KEYS) {
        app.keyboard[event->keysym.scancode] = 0;
    }
}

static void initPlayer() {
    player = (Entity*)malloc(sizeof(Entity));
    memset(player, 0, sizeof(Entity));
    stage.fighterTail->next = player;
    stage.fighterTail = player;

    player->health = 1;
    player->x = 100;
    player->y = 100;
    player->texture = loadTexture("gfx/player.png");
    SDL_QueryTexture(player->texture, NULL, NULL, &player->w, &player->h);

    player->reload = 0;
    player->side = SIDE_PLAYER;
}

static void doPointsPods(void){
    Entity* e, * prev;

    prev = &stage.pointsHead;

    for (e = stage.pointsHead.next; e != NULL; e = e->next) {
        if (e->x < 0) {
            e->x = 0;
            e->dx = -e->dx;
        }

        if (e->x + e->w > SCREEN_WIDTH) {
            e->x = SCREEN_WIDTH - e->w;
            e->dx = -e->dx;
        }

        if (e->y < 0) {
            e->y = 0;
            e->dy = -e->dy;
        }

        if (e->y + e->h > SCREEN_HEIGHT) {
            e->y = SCREEN_HEIGHT - e->h;
            e->dy = -e->dy;
        }

        e->x += e->dx;
        e->y += e->dy;

        if (player != NULL && collision(e->x, e->y, e->w, e->h, player->x, player->y, player->w, player->h)) {
            e->health = 0;

            stage.score++;

            highscore = fmax(stage.score, highscore);

            playSound(SND_POINTS, CH_POINTS);
        }

        if (--e->health <= 0) {
            if (e == stage.pointsTail) {
                stage.pointsTail = prev;
            }

            prev->next = e->next;
            free(e);
            e = prev;
        }

        prev = e;
    }
}

static void addPointsPod(int x, int y) {
    Entity* e;
    SDL_Rect dest;

    e = (Entity*)malloc(sizeof(Entity));
    memset(e, 0, sizeof(Entity));
    stage.pointsTail->next = e;
    stage.pointsTail = e;

    e->x = x;
    e->y = y;
    e->dx = -(rand() % 5);
    e->dy = (rand() % 5) - (rand() % 5);
    e->health = FPS * 10;
    e->texture = pointsTexture;

    SDL_QueryTexture(e->texture, NULL, NULL, &dest.w, &dest.h);

    e->w = dest.w;
    e->h = dest.h;

    e->x -= e->w / 2;
    e->y -= e->h / 2;
}

static void fireBullet(void) {
    Entity* bullet;
    SDL_Rect dest;

    bullet = (Entity*)malloc(sizeof(Entity));
    memset(bullet, 0, sizeof(Entity));
    stage.bulletTail->next = bullet;
    stage.bulletTail = bullet;

    bullet->x = player->x;
    bullet->y = player->y;
    bullet->dx = PLAYER_BULLET_SPEED;
    bullet->health = 1;
    bullet->texture = bulletTexture;
    SDL_QueryTexture(bullet->texture, NULL, NULL, &dest.w, &dest.h);

    bullet->w = dest.w;
    bullet->h = dest.h;
    bullet->x += (player->w / 2);
    bullet->y += (player->h / 2) - (bullet->h / 2);

    bullet->side = SIDE_PLAYER;

    player->reload = 8;
}

static void addExplosions(int x, int y, int num) {
    Explosion* e;
    int i;

    for (i = 0; i < num; i++) {
        e = (Explosion*)malloc(sizeof(Explosion));
        memset(e, 0, sizeof(Explosion));
        stage.explosionTail->next = e;
        stage.explosionTail = e;

        e->x = x + (rand() % 32) - (rand() % 32);
        e->y = y + (rand() % 32) - (rand() % 32);
        e->dx = (rand() % 10) - (rand() % 10);
        e->dy = (rand() % 10) - (rand() % 10);

        e->dx /= 10;
        e->dy /= 10;

        switch (rand() % 4) {
        case 0:
            e->r = 255;
            break;
        case 1:
            e->r = 255;
            e->g = 128;
            break;
        case 2:
            e->r = 255;
            e->g = 255;
            break;
        default:
            e->r = 255;
            e->g = 255;
            e->b = 255;
            break;
        }

        e->a = rand() % FPS * 3;
    }
}

static void addDebris(Entity* e) {
    Debris* d;
    int x, y, w, h;

    w = e->w / 2;
    h = e->h / 2;

    for (y = 0; y <= h; y += h) {
        for (x = 0; x <= w; x += w) {
            d = (Debris*)malloc(sizeof(Debris));
            memset(d, 0, sizeof(Debris));

            stage.debrisTail->next = d;
            stage.debrisTail = d;

            d->x = e->x + (e->w / 2);
            d->y = e->y + (e->h / 2);
            d->dx = (rand() % 5) - (rand() % 5);
            d->dy = -(5 + (rand() % 12));
            d->life = FPS * 2;
            d->texture = e->texture;

            d->rect.x = x;
            d->rect.y = y;
            d->rect.w = w;
            d->rect.h = h;
        }
    }
}

static int bulletHitFighter(Entity* b) {
    Entity* e;
    int result = 0;

    for (e = stage.fighterHead.next; e != NULL; e = e->next) {
        if (e->side != b->side && collision(b->x, b->y, b->w, b->h, e->x, e->y, e->w, e->h)) {
            b->health = 0;
            e->health = 0;

            if (e == player) {
                playSound(SND_PLAYER_DIE, CH_PLAYER);
            }

            else {
                addPointsPod(e->x + e->w / 2, e->y + e->h / 2);

                playSound(SND_ALIEN_DIE, CH_ANY);
            }

            addExplosions(e->x, e->y, 32);
            addDebris(e);

            result = 1;
        }
    }

    return result;
}


static void doBullets(void) {
    Entity* b, * prev;

    prev = &stage.bulletHead;

    for (b = stage.bulletHead.next; b != NULL; b = b->next) {
        b->x += b->dx;
        b->y += b->dy;

        if (bulletHitFighter(b) || b->x < -b->w || b->y < -b->h || b->x > SCREEN_WIDTH || b->y > SCREEN_HEIGHT) {
            if (b == stage.bulletTail) {
                stage.bulletTail = prev;
            }

            prev->next = b->next;
            free(b);
            b = prev;
        }

        prev = b;

        if (bulletHitFighter(b)) {
            addExplosions(b->x, b->y, 10);
            addDebris(b);
        }
    }
}

static void doPlayer(void) {
    if (player != NULL) {
        player->dx = 0;
        player->dy = 0;

        if (player->reload > 0) {
            player->reload--;
        }

        if (app.keyboard[SDL_SCANCODE_UP]) {
            player->dy = -PLAYER_SPEED;
        }
        if (app.keyboard[SDL_SCANCODE_DOWN]) {
            player->dy = PLAYER_SPEED;
        }
        if (app.keyboard[SDL_SCANCODE_LEFT]) {
            player->dx = -PLAYER_SPEED;
        }
        if (app.keyboard[SDL_SCANCODE_RIGHT]) {
            player->dx = PLAYER_SPEED;
        }
        if (app.keyboard[SDL_SCANCODE_LCTRL] && player->reload <= 0) {
            playSound(SND_PLAYER_FIRE, CH_PLAYER);
        }
        if (app.keyboard[SDL_SCANCODE_LCTRL] && player->reload <= 0) {
            fireBullet();
        }
    }
}

static void doFighters(void) {
    Entity* e, * prev;

    prev = &stage.fighterHead;

    for (e = stage.fighterHead.next; e != NULL; e = e->next) {
        e->x += e->dx;
        e->y += e->dy;

        if (e != player && e->x < -e->w) {
            e->health = 0;
        }

        if (e->health == 0) {
            if (e == player) {
                player = NULL;
            }

            if (e == stage.fighterTail) {
                stage.fighterTail = prev;
            }

            addExplosions(e->x, e->y, 10);
            addDebris(e);

            prev->next = e->next;
            free(e);
            e = prev;
        }

        prev = e;
    }
}

static void fireAlienBullet(Entity* e) {
    Entity* bullet;

    bullet = (Entity*)malloc(sizeof(Entity));
    memset(bullet, 0, sizeof(Entity));
    stage.bulletTail->next = bullet;
    stage.bulletTail = bullet;

    bullet->x = e->x;
    bullet->y = e->y;
    bullet->health = 1;
    bullet->texture = alienBulletTexture;
    bullet->side = SIDE_ALIEN;
    SDL_QueryTexture(bullet->texture, NULL, NULL, &bullet->w, &bullet->h);

    bullet->x += (e->w / 2) - (bullet->w / 2);
    bullet->y += (e->h / 2) - (bullet->h / 2);

    calcSlope(player->x + (player->w / 2), player->y + (player->h / 2), e->x, e->y, &bullet->dx, &bullet->dy);

    bullet->dx *= ALIEN_BULLET_SPEED;
    bullet->dy *= ALIEN_BULLET_SPEED;

    e->reload = (rand() % FPS * 2);
}

static void doEnemies(void) {
    Entity* e;
    for (e = stage.fighterHead.next; e != NULL; e = e->next) {
        if (e != player && player != NULL && --e->reload <= 0) {
            fireAlienBullet(e);

            playSound(SND_ALIEN_FIRE, CH_ALIEN_FIRE);
        }
    }
}

static void spawnEnemies(void) {
    Entity* enemy;

    if (--enemySpawnTimer <= 0) {
        enemy = (Entity*)malloc(sizeof(Entity));
        memset(enemy, 0, sizeof(Entity));
        stage.fighterTail->next = enemy;
        stage.fighterTail = enemy;

        enemy->x = SCREEN_WIDTH;
        enemy->y = rand() % SCREEN_HEIGHT;

        enemy->texture = enemyTexture;
        SDL_QueryTexture(enemy->texture, NULL, NULL, &enemy->w, &enemy->h);

        enemy->dx = -(2 + (rand() % 4));

        enemy->side = SIDE_ALIEN;

        enemy->health = 1;

        enemy->reload = FPS * (1 + (rand() % 3));

        enemySpawnTimer = 30 + (rand() % 60);
    }
}

static void initStarfield(void) {
    int i;

    for (i = 0; i < MAX_STARS; i++) {
        stars[i].x = rand() % SCREEN_WIDTH;
        stars[i].y = rand() % SCREEN_HEIGHT;
        stars[i].speed = 1 + rand() % 8;
    }
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

    while (stage.pointsHead.next){
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

    stage.score = 0;

    initPlayer();
    initStarfield();

    enemySpawnTimer = 0;

    stageResetTimer = FPS * 3;
}

static void clipPlayer(void) {
    if (player != NULL) {
        if (player->x < 0) {
            player->x = 0;
        }

        if (player->y < 0) {
            player->y = 0;
        }

        if (player->x > SCREEN_WIDTH - player->w) {
            player->x = SCREEN_WIDTH - player->w;
        }

        if (player->y > SCREEN_HEIGHT - player->h) {
            player->y = SCREEN_HEIGHT - player->h;
        }
    }
}

static void doBackground(void) {
    if (--backgroundX < -SCREEN_WIDTH) {
        backgroundX = 0;
    }
}

static void doStarfield(void) {
    int i;

    for (i = 0; i < MAX_STARS; i++) {
        stars[i].x -= stars[i].speed;

        if (stars[i].x < 0) {
            stars[i].x = SCREEN_WIDTH + stars[i].x;
        }
    }
}

static void doExplosions(void) {
    Explosion* e, * prev;

    prev = &stage.explosionHead;

    for (e = stage.explosionHead.next; e != NULL; e = e->next) {
        e->x += e->dx;
        e->y += e->dy;

        if (--e->a <= 0) {
            if (e == stage.explosionTail) {
                stage.explosionTail = prev;
            }

            prev->next = e->next;
            free(e);
            e = prev;
        }

        prev = e;
    }
}

static void doDebris(void) {
    Debris* d, * prev;
    prev = &stage.debrisHead;

    for (d = stage.debrisHead.next; d != NULL; d = d->next) {
        d->x += d->dx;
        d->y += d->dy;

        d->dy += 0.5;

        if (--d->life <= 0) {
            if (d == stage.debrisTail) {
                stage.debrisTail = prev;
            }

            prev->next = d->next;
            free(d);
            d = prev;
        }

        prev = d;
    }
}




static void logic(void) {
    doBackground();

    doStarfield();

    doPlayer();

    doEnemies();

    doFighters();

    doBullets();

    doExplosions();

    doDebris();

    doPointsPods();

    spawnEnemies();

    clipPlayer();

    if (player == NULL && --stageResetTimer <= 0) {
        resetStage();
    }
}

static void drawBullets(void) {
    Entity* b;
    for (b = stage.bulletHead.next; b != NULL; b = b->next) {
        blit(b->texture, b->x, b->y);
    }
}

static void drawFighters(void) {
    Entity* e;

    for (e = stage.fighterHead.next; e != NULL; e = e->next) {
        blit(e->texture, e->x, e->y);
    }
}

static void drawBackground(void) {
    SDL_Rect dest;
    int x;

    for (x = backgroundX; x < SCREEN_WIDTH; x += SCREEN_WIDTH) {
        dest.x = x;
        dest.y = 0;
        dest.w = SCREEN_WIDTH;
        dest.h = SCREEN_HEIGHT;

        SDL_RenderCopy(app.renderer, background, NULL, &dest);
    }
}

static void drawStarfield(void) {
    int i, c;

    for (i = 0; i < MAX_STARS; i++) {
        c = 32 * stars[i].speed;

        SDL_SetRenderDrawColor(app.renderer, c, c, c, 255);

        SDL_RenderDrawLine(app.renderer, stars[i].x, stars[i].y, stars[i].x + 3, stars[i].y);
    }
}

static void drawDebris(void) {
    Debris* d;

    for (d = stage.debrisHead.next; d != NULL; d = d->next) {
        blitRect(d->texture, &d->rect, d->x, d->y);
    }
}

static void drawExplosions(void) {
    Explosion* e;

    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_ADD);
    SDL_SetTextureBlendMode(explosionTexture, SDL_BLENDMODE_ADD);

    for (e = stage.explosionHead.next; e != NULL; e = e->next) {
        SDL_SetTextureColorMod(explosionTexture, e->r, e->g, e->b);
        SDL_SetTextureAlphaMod(explosionTexture, e->a);

        blit(explosionTexture, e->x, e->y);
    }

    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
}

void drawText(int x, int y, int r, int g, int b, std::string outText) {
    int len = outText.length();
    std::string upperText = outText;
    std::transform(outText.begin(), outText.end(), outText.begin(), ::toupper);

    SDL_Rect rect;

    rect.w = GLYPH_WIDTH;
    rect.h = GLYPH_HEIGHT;
    rect.y = 0;

    SDL_SetTextureColorMod(fontTexture, r, g, b);

    for (int i = 0; i < len; i++) {
        if (outText[i] >= ' ' && outText[i] <= 'Z') {
            rect.x = (outText[i] - ' ') * GLYPH_WIDTH;
            blitRect(fontTexture, &rect, x, y);
            x += GLYPH_WIDTH;
        }
    }
}


std::string numberfill(int a) {
    if (a >= 100) {
        return std::to_string(a);
    }
    if (a < 100 && a > 9) {
        return "0" + std::to_string(a);
    }

    if (a < 10) {
        return "00" + std::to_string(a);
    }
}

static void drawHud(void) {
    drawText(20, 10, 255, 255, 255, "SCORE: " + numberfill(stage.score));

    if (stage.score > 0 && stage.score == highscore) {
        drawText(980, 10, 0, 255, 0, "HIGH SCORE: " + numberfill(highscore));
    }

    else {
        drawText(980, 10, 255, 255, 255, "HIGH SCORE: " + numberfill(highscore));
    }
}

static void drawPointsPods(void) {
    Entity* e;

    for (e = stage.pointsHead.next; e != NULL; e = e->next) {
        blit(e->texture, e->x, e->y);
    }
}


static void draw(void) {
    drawBackground();

    drawStarfield();

    drawPointsPods();

    drawFighters();

    drawBullets();

    drawDebris();

    drawExplosions();

    drawHud();
}



void initStage(void) {
    app.delegate.logic = logic;
    app.delegate.draw = draw;

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.bulletTail = &stage.bulletHead;

    stage.explosionTail = &stage.explosionHead;
    stage.debrisTail = &stage.debrisHead;

    initPlayer();

    bulletTexture = loadTexture("gfx/playerBullet.png");
    enemyTexture = loadTexture("gfx/enemy.png");

    enemySpawnTimer = 0;
    alienBulletTexture = loadTexture("gfx/alienBullet.png");
    playerTexture = loadTexture("gfx/player.png");

    background = loadTexture("gfx/background.png");
    explosionTexture = loadTexture("gfx/explosion.png");

    pointsTexture = loadTexture("gfx/points.png");

    loadMusic;

    playMusic(1);

    resetStage();
}

static void capFrameRate(long* then, float* remainder) {
    long wait, frameTime;

    wait = 16 + *remainder;

    *remainder -= (int)*remainder;

    frameTime = SDL_GetTicks() - *then;

    wait -= frameTime;

    if (wait < 1) {
        wait = 1;
    }

    SDL_Delay(wait);

    *remainder += 0.667;

    *then = SDL_GetTicks();
}

void doInput(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            exit(0);
            break;
        case SDL_KEYDOWN:
            doKeyDown(&event.key);
            break;
        case SDL_KEYUP:
            doKeyUp(&event.key);
            break;
        default:
            break;
        }
    }
}


void cleanup(void) {
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
}


int main(int argc, char* argv[]) {
    long then;
    float remainder;

    memset(&app, 0, sizeof(App));

    initSDL();

    atexit(cleanup);

    initStage();

    initSounds();

    initFonts();

    then = SDL_GetTicks();

    remainder = 0;

    highscore = 0;

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