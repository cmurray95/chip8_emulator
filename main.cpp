#include "SDL.h"
#include "crisp8.h"
#include <stdio.h>
#include <iostream>

//Screen resolution
constexpr auto SCREEN_WIDTH  = 1024;
constexpr auto SCREEN_HEIGHT = 576 ;

//Surface sizes
constexpr auto SURFACE_WIDTH = 64;
constexpr auto SURFACE_HEIGHT = 32;

int main(int argc, char* argv[])
{
	bool quit = false;
	SDL_Event event;
	//Create window for displayy
	SDL_Window* window = nullptr;
	//Renderer to draw to
	SDL_Renderer* renderer = nullptr;
	//Texture that will be output to renderer
	SDL_Texture* texture = nullptr;
	crisp8 emulator;
	
	//Initiate SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to initialize SDL! Error: %s\n", SDL_GetError());
		return 1;
	}

	//Enable texture filtering
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		printf("Linear filtering is not enabled!");
	}

	//Initialize window
	window = SDL_CreateWindow("crisp 8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		printf("Error creating window! Error: %s\n", SDL_GetError());
		return 1;
	}


	//Initialize Renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr) {
		printf("Failed to create renderer! Error: %s\n", SDL_GetError());
		quit = true;
	}
	else {
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	}

	//Initialize texture
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

	if (!emulator.loadProgram("pong2.c8")) {
		return 1;
	}
	while (!quit) {
		emulator.cycle();
		if (emulator.drawFlag) {
			//Refreshe renderer
			SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			//Pixel to be drawn
			SDL_Rect pixel;
			//Draw to screen using SDL_Rects to mimic pixels
			int row = 0;
			for (int y = 0; y < 32; ++y) {
				for (int x = 0; x < 64; ++x) {
					pixel.x = x * (SCREEN_WIDTH / 64);
					pixel.y = y * (SCREEN_HEIGHT / 32);
					pixel.w = 10;
					pixel.h = 10;
					row = y * 64;
					if (emulator.graphics[x + row] == 1) {
						SDL_RenderFillRect(renderer, &pixel);
					}
				}
			}
			SDL_RenderPresent(renderer);
			emulator.drawFlag = false;
		}
		SDL_PollEvent(&event);



		//Test for program input
		switch (event.type) {
		case SDL_QUIT:
			quit = true;
			break;
		}
	}

	//Destroy window and clear memory
	SDL_DestroyWindow(window);
	SDL_Quit();
	printf("End");
	return 0;
}
