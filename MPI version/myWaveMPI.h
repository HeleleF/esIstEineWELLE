/**
 * @file myWaveMPI.h
 * @author Chris Rebbelin s0548921
 * @date 2018-07-01
 * @brief contains the main MPI program 
 */

#ifndef __WAVE_MPI_H_
#define __WAVE_MPI_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "coreMPI.h"

/**
  * Visualizes the wave equation with the given
  * parameters with the SDL library.
  */
void doGraphics(void);

/**
  * Initializes all needed SDL variables.
  *
  * @param win pointer to the SDL_Window
  * @param ren pointer to the SDL_Renderer
  * @param fon pointer to the TTF_Font
  */
void initSdlVars(SDL_Window** win, SDL_Renderer** ren, TTF_Font** fon);

#endif