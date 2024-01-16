#include <iostream>
#include <stdio.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <string.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define PLAYER_SPEED 4
#define PLAYER_BULLET_SPEED 16
#define ALIEN_BULLET_SPEED 8

#define MAX_KEYBOARD_KEYS 350

#define SIDE_PLAYER 0
#define SIDE_ALIEN 1

#define FPS 60

typedef struct {
    void (*logic)(void);
    void (*draw)(void);
} Delegate;

typedef struct {
    SDL_Renderer* renderer;
    SDL_Window* window;
    Delegate delegate;
    int keyboard[MAX_KEYBOARD_KEYS];
} App;

typedef struct Entity {
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
} Entity;

typedef struct {
    Entity fighterHead, * fighterTail;
    Entity bulletHead, * bulletTail;
} Stage;

App app;
Entity* player;
Entity* bullet;
Entity* enemy;
Stage stage;
SDL_Texture* bulletTexture;
SDL_Texture* enemyTexture;
SDL_Texture* alienBulletTexture;
SDL_Texture* playerTexture;
int enemySpawnTimer;
int stageResetTimer;

int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (fmax(x1, x2) < fmin(x1 + w1, x2 + w2)) && (fmax(y1, y2) < fmin(y1 + h1, y2 + h2));
}

void calcSlope(int x1, int y1, int x2, int y2, float *dx, float *dy) {
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


void initSDL(void) {
    int rendererFlags, windowFlags;

    rendererFlags = SDL_RENDERER_ACCELERATED;

    windowFlags = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Couldn't initialize SDL: %d\n", SDL_GetError());
        exit(1);
    }

    app.window = SDL_CreateWindow("Shooter 01", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);
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

static int bulletHitFighter(Entity* b) {
    Entity* e;
    int result = 0;

    for (e = stage.fighterHead.next; e != NULL; e = e->next) {
        if (e->side != b->side && collision(b->x, b->y, b->w, b->h, e->x, e->y, e->w, e->h)) {
            b->health = 0;
            e->health = 0;

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

    calcSlope((int)(player->x + (player->w / 2)), (int)(player->y + (player->h / 2)), (int)e->x, (int)e->y, &bullet->dx, &bullet->dy);

    bullet->dx *= ALIEN_BULLET_SPEED;
    bullet->dy *= ALIEN_BULLET_SPEED;
    
    e->reload = (rand() % FPS * 2);
}

static void doEnemies(void) {
    Entity* e;

    for (e = stage.fighterHead.next; e != NULL; e = e->next) {
        if (e != player && player != NULL) {
            --e->reload;
            if (e->reload <= 0) {
                fireAlienBullet(e);
            }
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

static void resetStage(void) {
    Entity* e;
    
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

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.bulletTail = &stage.bulletHead;

    initPlayer();

    enemySpawnTimer = 0;

    stageResetTimer = FPS * 2;
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

static void logic(void) {
    doPlayer();

    doEnemies();

    doFighters();

    doBullets();

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

static void draw(void) {
    drawFighters();

    drawBullets();
}



void initStage(void) {
    app.delegate.logic = logic;
    app.delegate.draw = draw;

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.bulletTail = &stage.bulletHead;

    initPlayer();

    bulletTexture = loadTexture("gfx/playerBullet.png");
    enemyTexture = loadTexture("gfx/enemy.png");

    enemySpawnTimer = 0;
    alienBulletTexture = loadTexture("gfx/alienBullet.png");
    playerTexture = loadTexture("gfx/player.png");

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

    then = SDL_GetTicks();

    remainder = 0;

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