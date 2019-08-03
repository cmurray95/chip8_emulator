#include "SDL.h"
#include "crisp8.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow
	("An SDL2 window", // window's title
		10, 25, // coordinates on the screen, in pixels, of the window's upper left corner
		640, 480, // window's length and height in pixels  
		SDL_WINDOW_OPENGL);

	//SDL_Delay(3000); // window lasts 3 seconds

	crisp8 crisp8;
	
	if (!crisp8.loadProgram("invaders.c8")) {
		return 1;
	}
	while (true) {
		crisp8.cycle();
	}
	return 0;
}