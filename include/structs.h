#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL2/SDL.h>
#include "defs.h"

// -- Định nghĩa các cấu trúc dữ liệu --

struct Texture {
    char name[MAX_NAME_LENGTH];
    SDL_Texture* texture;
    Texture* next;
};

struct Delegate {
    void (*logic)(void);
    void (*draw)(void);
};

struct App {
    SDL_Renderer* renderer;
    SDL_Window* window;
    Delegate delegate;
    Texture textureHead;
    Texture* textureTail;
    int keyboard[MAX_KEYBOARD_KEYS];
    char inputText[MAX_LINE_LENGTH];
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

struct Highscore {
    int recent;
    int score;
    char name[MAX_SCORE_NAME_LENGTH];
};

struct Highscores {
    Highscore highscore[NUM_HIGHSCORES];
};

#endif // STRUCTS_H