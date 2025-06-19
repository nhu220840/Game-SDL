#include <string.h>
#include <SDL.h>

#include "input.h"
#include "globals.h"

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

void doInput(void) {
    SDL_Event event;

    // Xóa bộ đệm văn bản ở mỗi vòng lặp
    app.inputText[0] = '\0';

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
            case SDL_TEXTINPUT:
                // Sao chép văn bản nhập vào biến toàn cục app.inputText
                strncpy(app.inputText, event.text.text, MAX_LINE_LENGTH - 1);
                break;
            default:
                break;
        }
    }
}