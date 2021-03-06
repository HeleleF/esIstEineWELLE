/**
 * @file myWave.h
 * @author Chris Rebbelin s0548921
 * @date 2018-07-29
 * @brief header file for the main program in @c myWave.c
 *
 * @details This file contains all needed definitions and includes
 * for the main program implemented in @c myWave.c.
 */

#ifndef __WAVE_H_
#define __WAVE_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "core.h"

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
 * @def HOLD_TOLERANCE
 * 
 * @brief Represents the tolerance value for "holding" a point
 * 
 * If the left mouse-button is clicked, its coordinates are checked:
 * First it is checked if the x-Coordinate is actually on the x-Axis.
 * Then the y-Coordinate of the wave at that x is taken. If the absolute
 * difference of this y and the mouse-y is less than HOLD_TOLERANCE, the
 * Point at x is marked as "hold". 
 * This was implemented because it is not possible to accurately click on
 * the exact wave line pixel.
 */
#define HOLD_TOLERANCE 20

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
  * @brief Initializes the main SDL variables
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

#endif //__WAVE_H_