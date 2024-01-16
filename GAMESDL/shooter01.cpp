#include <iostream>
#include <SDL.h>

#define SCREEN_WIDTH   1280
#define SCREEN_HEIGHT  720

typedef struct {
	SDL_Renderer* renderer;
	SDL_Window* window;
} App;

App app;

void initSDL(void){
	int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED;

	windowFlags = 0;

	if (SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	app.window = SDL_CreateWindow("Shooter 01", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);

	if (!app.window){
		printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);

	if (!app.renderer){
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}
}

void doInput(void){
	SDL_Event event;

	while (SDL_PollEvent(&event)){
		switch (event.type){
		case SDL_QUIT:
			exit(0);
			break;

		default:
			break;
		}
	}
}

void prepareScene(void){
	SDL_SetRenderDrawColor(app.renderer, 96, 128, 255, 255);
	SDL_RenderClear(app.renderer);
}

void presentScene(void){
	SDL_RenderPresent(app.renderer);
}

void cleanup(void) {
	SDL_DestroyRenderer(app.renderer);
	SDL_DestroyWindow(app.window);
	SDL_Quit();
}


int main(int argc, char* argv[]){
	memset(&app, 0, sizeof(App));

	initSDL();

	atexit(cleanup);

	while (1){
		prepareScene();

		doInput();

		presentScene();

		SDL_Delay(16);
	}

	return 0;
}