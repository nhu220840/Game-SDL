#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

SDL_Texture* loadTexture(const char* filename);
void loadMusic(const char* filename);
void playMusic(int loop);
void playSound(int id, int channel);
void capFrameRate(long* then, float* remainder);
int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
void calcSlope(int x1, int y1, int x2, int y2, float* dx, float* dy);
std::string numberfill(int a);

#endif // UTILS_H