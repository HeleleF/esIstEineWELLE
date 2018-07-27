/**
 * @file myWaveMPI.c
 * @author Chris Rebbelin s0548921
 * @date 2018-07-01
 * @brief This file contains the main MPI program
 * 
 * This file implements the main program and visualisation to the user.
 */

#include "myWaveMPI.h"


const int MASTER = 0;

int id, numberOfProcesses;


void initSdlVars(SDL_Window **win, SDL_Renderer **ren, TTF_Font **fon)
{

    // init main sdl
    if (SDL_Init(SDL_INIT_EVERYTHING) == EXIT_FAILURE)
    {

        printf("SDL_Init Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    };

    // init text specific sdl
    if (TTF_Init() == EXIT_FAILURE)
    {

        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    };

    // init image specific sdl
    if (IMG_Init((IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG)))
    {

        printf("Initialising SDL_image failed. Error: \n%s\n", IMG_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // create the window
    *win = SDL_CreateWindow("1D Wave equation MPI - psys18", 0, 0, MY_WINDOW_WIDTH, MY_WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (*win == NULL)
    {

        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // create the renderer
    *ren = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED);
    if (*ren == NULL)
    {

        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*win);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // load the font
    *fon = TTF_OpenFont(DEFAULT_FONT_PATH, DEFAULT_FONT_SIZE);
    if (*fon == NULL)
    {

        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(*ren);
        SDL_DestroyWindow(*win);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // create window icon
    SDL_Surface *icon = IMG_Load(ICON_PATH);
    if (icon == NULL)
    {

        printf("SDL_Surface Error: %s\n", SDL_GetError());
        TTF_CloseFont(*fon);
        SDL_DestroyRenderer(*ren);
        SDL_DestroyWindow(*win);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // set window icon
    SDL_SetWindowIcon(*win, icon);
}

void doGraphics()
{

            // the window we'll be rendering to
        SDL_Window *gWindow = NULL;

        // the window renderer
        SDL_Renderer *gRenderer = NULL;

        // the text font
        TTF_Font *font = NULL;

SDL_Rect textrect = {0, 0, 0, 0};
        

        int w = MY_WINDOW_WIDTH;
        int h = MY_WINDOW_HEIGHT;

        // constant setting values
        const int tpoints = getTpoints();
        const int npoints = getNpoints();
        const double lambda = getLambda();

        // time counter
        int currentTimeStep = 1;

        // init status texture
        const SDL_Color textColor = {0, 255, 0};
        int texW, texH;

        SDL_Surface *textSurface = NULL;
        SDL_Texture *pauseTexture = NULL;

        // status flags
            int run = 1;
    int doPause = 0;
        int showAxis = 0;

        // damping factor
        double z = 1.0;

        // current wave values
        double *currentSimulationStep;
        double prev, now;

        // variables for framerate
        const int fps = 21;
        const Uint32 ticksPerFrame = 1000 / fps;

        static Uint32 lastFrameTick = 0;
        Uint32 currentFrameTick, elapsedTicks;

    if (id == MASTER)
    {

        initSdlVars(&gWindow, &gRenderer, &font);

                textSurface = TTF_RenderText_Solid(font, "Running", textColor);
        pauseTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);

        SDL_QueryTexture(pauseTexture, NULL, NULL, &texW, &texH);
                                textrect.x = w - texW - TEXT_OFFSET;
                        textrect.y = h - texH - TEXT_OFFSET;
                                textrect.w = texW;
                        textrect.h = texH;

    }

    // main event loop
    while (run)
    {

        if (id == MASTER)
        {
            static SDL_Event event;

            // poll for sdl events and handle them
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {

                case SDL_QUIT:
                    run = 0;
                    break;

                case SDL_KEYUP:

                    switch (event.key.keysym.sym)
                    {

                    case SDLK_q:
                    case SDLK_ESCAPE:
                        run = 0;
                        break;

                    case SDLK_a:
                        showAxis = !showAxis;
                        break;

                    case SDLK_p:
                        if (doPause)
                        {
                            textSurface = TTF_RenderText_Solid(font, "Running", textColor);
                        }
                        else
                        {
                            textSurface = TTF_RenderText_Solid(font, "Paused", textColor);
                        }

                        doPause = !doPause;
                        pauseTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);

                        SDL_QueryTexture(pauseTexture, NULL, NULL, &texW, &texH);
                        textrect.w = texW;
                        textrect.h = texH;

                        break;

                    case SDLK_r:
                        resetWave();
                        currentTimeStep = 1;

                        if (doPause)
                        {
                            doPause = 0;

                            textSurface = TTF_RenderText_Solid(font, "Running", textColor);
                            pauseTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);

                            SDL_QueryTexture(pauseTexture, NULL, NULL, &texW, &texH);
                            textrect.w = texW;
                            textrect.h = texH;
                        }

                        break;

                    default:
                        break;
                    }
                    break;

                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        SDL_Surface *content = SDL_GetWindowSurface(gWindow);

                        SDL_GetWindowSize(gWindow, &w, &h);

                        SDL_Rect rect = {0, 0, w, h};
                        SDL_FillRect(content, &rect, 0x000000FF);

                        SDL_UpdateWindowSurface(gWindow);

                        textrect.x = w - texW - TEXT_OFFSET;
                        textrect.y = h - texH - TEXT_OFFSET;
                    }
                    break;

                default:
                    break;
                }
            }

            // framerate
            currentFrameTick = SDL_GetTicks();
            if (lastFrameTick == 0)
            {
                lastFrameTick = SDL_GetTicks();
            }

            // clear the screen
            SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
            SDL_RenderClear(gRenderer);

            // draw x and y axis if needed
            if (showAxis)
            {
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderDrawLine(gRenderer, WIDTH_OFFSET, (h / 2) - Y_AXIS_LENGTH, WIDTH_OFFSET, (h / 2) + Y_AXIS_LENGTH);
                SDL_RenderDrawLine(gRenderer, WIDTH_OFFSET, h / 2, 2 * WIDTH_OFFSET + npoints, h / 2);
            }
        }

		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Bcast(&doPause, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

        // simulate one time step
        if (!doPause)
        {
            simulateOneTimeStep();

            collectWave();
            MPI_Barrier(MPI_COMM_WORLD);

            if (id == MASTER)
            {
                currentTimeStep++;
            }
        }

        if (id == MASTER)
        {
            // get the current values and draw them
            currentSimulationStep = getStep();

            // apply damping
            z = exp((double) -currentTimeStep * lambda);

            for (int l = 1; l < npoints; ++l)
            {

                prev = currentSimulationStep[l - 1] * z;
                now = currentSimulationStep[l] * z;

                // set color to green and draw line between point and its left neighbor
                SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255);
                SDL_RenderDrawLine(gRenderer, l - 1 + WIDTH_OFFSET, prev + (h / 2), l + WIDTH_OFFSET, now + (h / 2));
            }

            // render all
            SDL_RenderCopy(gRenderer, pauseTexture, NULL, &textrect);
            SDL_RenderPresent(gRenderer);

            // check if time is over
            // if timesteps was set to 0, this results in an endless loop
            // because currentTimeStep starts at 1 and increases, while tpoints stays at 0
            if (currentTimeStep == tpoints)
            {
                run = 0;
            }

            // check framerate
            if ((currentFrameTick - lastFrameTick) >= ticksPerFrame)
            {
                lastFrameTick = currentFrameTick;
            }
            else
            {
                // "too fast", wait a little
                elapsedTicks = currentFrameTick - lastFrameTick;
                SDL_Delay(ticksPerFrame - elapsedTicks);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(&run, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    }

    if (id == MASTER)
    {
        // quit the program
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(pauseTexture);
        TTF_CloseFont(font);
        SDL_DestroyRenderer(gRenderer);
        SDL_DestroyWindow(gWindow);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
    }
    MPI_Barrier(MPI_COMM_WORLD);
}

int main(int argc, char **argv)
{

    // init mpi
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

    // set mpi error handler
    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    getUserInputOrConfig(argc, argv, id, numberOfProcesses);

    if (doBench())
    {
        performBenchmark();
        return EXIT_SUCCESS;
    }

    if (showGui())
    {

        initWaveConditions();

        if (id == MASTER)
        {
        printf("\n Controls:\n\tA\t\ttoggle axis\n");
        printf("\tP\t\tpause / continue the visualisation\n");
        printf("\tR\t\treset to initial sine wave\n");
        printf("\tQ / ESC\t\tquit the program\n");
}
        doGraphics();
    }
    else
    {

        double waveTime;

        initWaveConditions();
        waveTime = simulateNumberOfTimeSteps();

        if (id == MASTER)
        {
            printf("Elapsed wallclock time was %g seconds\n", waveTime);
        }
    }

    // cleanup
    finalizeWave();

    return EXIT_SUCCESS;
}
