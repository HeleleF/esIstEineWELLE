/**
 * @file myWaveMPI.h
 * @author Chris Rebbelin s0548921
 * @date 2018-07-29
 * @brief header file for the main program in @c myWaveMPI.c
 * 
 * @details This file contains all needed definitions and includes
 * for the main mpi program implemented in @c myWaveMPI.c.
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
 * @def DEFAULT_FONT_PATH
 * 
 * The default font used for the application
 */
#define DEFAULT_FONT_PATH "../Ubuntu-L.ttf"

/**
 * @def DEFAULT_FONT_SIZE
 * 
 * The default font size
 */
#define DEFAULT_FONT_SIZE 20

/**
 * @def ICON_PATH
 * 
 * The icon path for the application
 */
#define ICON_PATH "../psysIcon.png"

/**
  * @brief Initializes all needed SDL variables
  *
  * @param win pointer to the SDL_Window
  * @param ren pointer to the SDL_Renderer
  * @param fon pointer to the TTF_Font
  */
void initSdlVars(SDL_Window** win, SDL_Renderer** ren, TTF_Font** fon);

/**
  * @brief Frees all SDL variables
  *
  * @param win pointer to the SDL_Window
  * @param ren pointer to the SDL_Renderer
  * @param fon pointer to the TTF_Font
  * @param surf pointer to the SDL_Surface
  * @param tex pointer to the SDL_Texture
  */
void closeSdlVars(SDL_Window** win, SDL_Renderer** ren, TTF_Font** fon, SDL_Surface **surf, SDL_Texture **tex);

/**
  * @brief Visualizes the wave equation with the SDL library
  */
void doGraphics(void);

#endif //__WAVE_MPI_H_