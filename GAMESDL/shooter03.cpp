#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <string.h>

#define SCREEN_WIDTH   1280
#define SCREEN_HEIGHT  720

typedef struct {
	SDL_Renderer* renderer;
	SDL_Window* window;
	int up;
	int down;
	int left;
	int right;
} App;

typedef struct {
	int x;
	int y;
	SDL_Texture* texture;
} Entity;

App app;
Entity player;

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

void initSDL(void) {
	int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED;

	windowFlags = 0;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
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

void doKeyDown(SDL_KeyboardEvent* event){
	if (event->repeat == 0){
		if (event->keysym.scancode == SDL_SCANCODE_UP){
			app.up = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_DOWN){
			app.down = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_LEFT){
			app.left = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_RIGHT){
			app.right = 1;
		}
	}
}

void doKeyUp(SDL_KeyboardEvent* event) {
	if (event->repeat == 0) {
		if (event->keysym.scancode == SDL_SCANCODE_UP) {
			app.up = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_DOWN) {
			app.down = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_LEFT) {
			app.left = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_RIGHT) {
			app.right = 0;
		}
	}
}

void doInput(void){
	SDL_Event event;

	while (SDL_PollEvent(&event)){
		switch (event.type){
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

void prepareScene(void) {
	SDL_SetRenderDrawColor(app.renderer, 96, 128, 255, 255);
	SDL_RenderClear(app.renderer);
}

void presentScene(void) {
	SDL_RenderPresent(app.renderer);
}

void cleanup(void) {
	SDL_DestroyRenderer(app.renderer);
	SDL_DestroyWindow(app.window);
	SDL_Quit();
}


int main(int argc, char* argv[]) {
	memset(&app, 0, sizeof(App));

	initSDL();

	player.x = 100;
	player.y = 100;
	player.texture = loadTexture("gfx/player.png");

	atexit(cleanup);

	while (1) {
		prepareScene();

		doInput();

		if (app.up)	player.y -= 4;

		if (app.down) player.y += 4;

		if (app.left) player.x -= 4;

		if (app.right) player.x += 4;

		blit(player.texture, player.x, player.y);

		presentScene();

		SDL_Delay(16);
	}

	return 0;
}