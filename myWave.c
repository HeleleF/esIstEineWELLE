	/**
	 * @file myWave.c
	 *
	 * This is the main driver of the program, i.e.,
	 * the program, which is then used by the user.
	 */
#include <SDL2/SDL.h>

	#include <stdio.h>
	#include <stdlib.h>

	#include "core.h"

#define MEIN_WINDOW_WIDHT 1280
#define MEIN_WINDOW_HEIGHT 720




void doGraphics() {

	//The window we'll be rendering to
	SDL_Window* gWindow = NULL;

	//The window renderer
	SDL_Renderer* gRenderer = NULL;


	if(SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
	};

	//Create window
	gWindow = SDL_CreateWindow( "SDL Tutorial", 0, 0, MEIN_WINDOW_WIDHT, MEIN_WINDOW_HEIGHT, SDL_WINDOW_SHOWN );
	if(gWindow == NULL) {
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
	}

	// create rednerer
	gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
	if(gRenderer == NULL) {
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
	}

	const int fps = 10;
	const Uint32 ticksPerFrame = 1000 / fps; 

	const int tpoints = getTPOINTS(); 
	const int npoints = getNPOINTS();
	int howmanyt = 1;

	int run = 1;

	// event loop
	while(run) {

		SDL_Event event;
		
		while(SDL_PollEvent(&event)) {

			switch (event.type) {

				case SDL_QUIT: 
					run = 0; 
					break;

				case SDL_KEYUP: 

					if(event.key.keysym.sym == SDLK_q) 
						run = 0;
					break;

				default: 
					break;

			}
		}

		if (howmanyt == tpoints) {
			run = 0;
			break;
		}

		SDL_DisplayMode mode;
		int w;
		int h;
		static Uint32 lastFrameTick = 0;
		Uint32 currentFrameTick = SDL_GetTicks();
		if(lastFrameTick == 0) {
			lastFrameTick = SDL_GetTicks();
		}

		/*
		SDL_GetWindowDisplayMode(gWindow, &mode);
		w = mode.w;
		h = mode.h;*/

  /* Clear the screen */
		SDL_SetRenderDrawColor(gRenderer,255,0,0,255);
		SDL_RenderClear(gRenderer);

  // hier malen
		simulateOneTimeStep();
		howmanyt++;

		double* vals = getStep();

		for (int l = 1; l < npoints+1; ++l) {
			SDL_SetRenderDrawColor(gRenderer,255,255,0,255);

			SDL_RenderDrawLine(gRenderer, l-1,vals[l-1]+(MEIN_WINDOW_HEIGHT / 2), l, vals[l]+(MEIN_WINDOW_HEIGHT / 2));

		}

		SDL_RenderPresent(gRenderer);

		if((currentFrameTick - lastFrameTick) >= ticksPerFrame) {
			lastFrameTick = currentFrameTick;
		} else {
    //printf("Delay needed\n");
			Uint32 elapsedTicks = currentFrameTick - lastFrameTick;
			SDL_Delay(ticksPerFrame - elapsedTicks);
		}
	}

	  /* Quit the program */
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	SDL_Quit();

}



int main(int argc, char **argv)
{

	getUserInputOrConfig(argc, argv);

	int useGui = 1;

	initMeins();

	if (useGui) {
		doGraphics();

	} else {
		simulateNumberOfTimeSteps();
	}

	closeMeins();

	return EXIT_SUCCESS;
}
