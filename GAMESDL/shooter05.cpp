#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <string.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define PLAYER_SPEED 4
#define PLAYER_BULLET_SPEED 16

#define MAX_KEYBOARD_KEYS 350

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
Stage stage;
SDL_Texture* bulletTexture;

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

    player->x = 100;
    player->y = 100;
    player->texture = loadTexture("gfx/player.png");
    SDL_QueryTexture(player->texture, NULL, NULL, &player->w, &player->h);
}

static void fireBullet(void) {
    Entity *bullet;
    
    bullet = (Entity*)malloc(sizeof(Entity));
    memset(bullet, 0, sizeof(Entity));
    stage.bulletTail->next = bullet;
    stage.bulletTail = bullet;

    bullet->x = player->x;
    bullet->y = player->y;
    bullet->dx = PLAYER_BULLET_SPEED;
    bullet->health = 1;
    bullet->texture = bulletTexture;
    SDL_QueryTexture(bullet->texture, NULL, NULL, &bullet->w, &bullet->h);

    bullet->y += (player->h / 2) - (bullet->h / 2);

    player->reload = 8;
}

static void doBullets(void) {
    Entity *b, *prev;

    prev = &stage.bulletHead;

    for (b = stage.bulletHead.next; b != NULL; b = b->next) {
        b->x += b->dx;
        b->y += b->dy;

        if (b->x > SCREEN_WIDTH) {
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
    player->dx = player->dy = 0;
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
        player->dy = -PLAYER_SPEED;
    }
    if (app.keyboard[SDL_SCANCODE_RIGHT]) {
        player->dy = PLAYER_SPEED;
    }
    if (app.keyboard[SDL_SCANCODE_LCTRL] && player->reload == 0) {
        fireBullet();
    }

    player->x += player->dx;
    player->y += player->dy;
}

static void logic(void) {
    doPlayer();

    doBullets();
}

static void drawPlayer(void) {
    blit(player->texture, player->x, player->y);
}

static void drawBullets(void) {
    Entity *b;
    for (b = stage.bulletHead.next; b != NULL; b = b->next) {
        blit(b->texture, b->x, b->y);
    }
}

static void draw(void) {
    drawPlayer();

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