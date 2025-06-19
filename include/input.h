#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL_events.h>

void doInput(void);
void doKeyDown(SDL_KeyboardEvent* event);
void doKeyUp(SDL_KeyboardEvent* event);

#endif // INPUT_H