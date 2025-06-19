#include <math.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>

#include "utils.h"
#include "globals.h"

// Khai báo các hàm tĩnh chỉ được sử dụng trong tệp này
static void addTextureToCache(const char* name, SDL_Texture* sdlTexture);
static SDL_Texture* getTexture(const char* name);

SDL_Texture* loadTexture(const char* filename) {
    SDL_Texture* texture = getTexture(filename);
    if (texture == NULL) {
        texture = IMG_LoadTexture(app.renderer, filename);
        if (texture == NULL) {
            printf("ERROR: Failed to load texture %s! SDL_image Error: %s\n", filename, IMG_GetError());
        }
        addTextureToCache(filename, texture);
    }
    return texture;
}

static void addTextureToCache(const char* name, SDL_Texture* sdlTexture) {
    Texture* texture = (Texture*)malloc(sizeof(Texture));
    memset(texture, 0, sizeof(Texture));
    if (app.textureTail == NULL) {
        app.textureHead.next = texture;
        app.textureTail = texture;
    } else {
        app.textureTail->next = texture;
        app.textureTail = texture;
    }
    strncpy(texture->name, name, MAX_NAME_LENGTH - 1);
    texture->name[MAX_NAME_LENGTH - 1] = '\0';
    texture->texture = sdlTexture;
}

static SDL_Texture* getTexture(const char* name) {
    for (Texture* t = app.textureHead.next; t != NULL; t = t->next) {
        if (strcmp(t->name, name) == 0) return t->texture;
    }
    return NULL;
}

void loadMusic(const char* filename) {
    if (music != NULL) {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
    }
    music = Mix_LoadMUS(filename);
    if (music == NULL) {
        printf("ERROR: Failed to load music %s! SDL_mixer Error: %s\n", filename, Mix_GetError());
    }
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2); // Giảm âm lượng nhạc nền một chút
}

void playMusic(int loop) { Mix_PlayMusic(music, (loop) ? -1 : 0); }
void playSound(int id, int channel) { Mix_PlayChannel(channel, sounds[id], 0); }

void capFrameRate(long* then, float* remainder) {
    long wait, frameTime;
    wait = 16 + *remainder;
    *remainder -= (int)*remainder;
    frameTime = SDL_GetTicks() - *then;
    wait -= frameTime;
    if (wait < 1) wait = 1;
    SDL_Delay(wait);
    *remainder += 0.667f;
    *then = SDL_GetTicks();
}

int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (fmax(x1, x2) < fmin(x1 + w1, x2 + w2)) && (fmax(y1, y2) < fmin(y1 + h1, y2 + h2));
}

void calcSlope(int x1, int y1, int x2, int y2, float* dx, float* dy) {
    int steps = fmax(abs(x1 - x2), abs(y1 - y2));
    if (steps == 0) { *dx = *dy = 0; return; }
    *dx = (float)(x1 - x2) / steps;
    *dy = (float)(y1 - y2) / steps;
}

std::string numberfill(int a) {
    char buf[10];
    if (a >= 100) {
        sprintf(buf, "%d", a);
    } else if (a >= 10) {
        sprintf(buf, "0%d", a);
    } else {
        sprintf(buf, "00%d", a);
    }
    return std::string(buf);
}