/**
 * @file myWaveMPI.h
 * @author Chris Rebbelin s0548921
 * @date 2018-07-01
 * @brief header file for the main program in @file myWaveMPI.c
 */

#ifndef __WAVE_MPI_H_
#define __WAVE_MPI_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "coreMPI.h"

/**
 * @def MY_WINDOW_WIDTH
 * 
 * Represents the default window width
 */
#define MY_WINDOW_WIDTH 1024

/**
 * @def MY_WINDOW_HEIGHT
 * 
 * Represents the default window height
 */
#define MY_WINDOW_HEIGHT 720

/**
 * @def WIDTH_OFFSET
 * 
 * Represents the offset from the left screen border
 */
#define WIDTH_OFFSET 20

/**
 * @def TEXT_OFFSET
 * 
 * Represents the offset for the status text box
 */
#define TEXT_OFFSET 20

/**
 * @def Y_AXIS_LENGTH
 * 
 * Represents the length of the y-Axis in pixel
 */
#define Y_AXIS_LENGTH 256

/**
  * @brief Visualizes the wave equation with the SDL library
  */
void doGraphics(void);

/**
  * @brief Initializes all needed SDL variables
  *
  * @param win pointer to the SDL_Window
  * @param ren pointer to the SDL_Renderer
  * @param fon pointer to the TTF_Font
  */
void initSdlVars(SDL_Window** win, SDL_Renderer** ren, TTF_Font** fon);

#endif //__WAVE_MPI_H_