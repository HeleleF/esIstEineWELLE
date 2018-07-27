/**
 * @file core.h
 * @author Chris Rebbelin s0548921
 * @date 2018-07-01
 * @brief header file for @file core.c
 * 
 * This file contains all needed definitions and includes
 * for the main calculation logic implemented in @file core.c.
 */

#ifndef __CORE_H_
#define __CORE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/**
 * @def _USE_MATH_DEFINES
 * 
 * Used for the constant M_PI from math.h.
 */
#define _USE_MATH_DEFINES
#include <math.h>

/**
 * @def MAX_POINTS
 * 
 * Represents the maximum number of discrete wave points
 */
#define MAX_POINTS 10000000

/**
 * @def MAX_LAMBDA
 * 
 * Represents the maximum value for the damping factor
 */
#define MAX_LAMBDA 0.1

/**
 * @def DEFAULT_SETTINGS_FILE_PATH
 * 
 * The default settings file used for configuration
 */
#define DEFAULT_SETTINGS_FILE_PATH "settings.txt"

/**
 * @def BENCHMARK_FILE
 * 
 * The default output file used for benchmark results
 */
#define BENCHMARK_FILE "benchResults.txt"


/**
  * @brief Calculates the initial sine wave values 
  *
  * @param x The x value
  * @return The value of the sine function at x
  */
double waveInitFunc(double x);

/**
  * @brief Reads the settings from file specified by a given file path
  *
  * @param configPath Path to a settings file
  */
void getFromSettingsFile(char *configPath);

/**
  * @brief Parses the settings from command line
  *
  * @param nargc argument count of the program
  * @param argv arguments of the program
  */
void getFromCmdLine(int nargc, char** argv);

/**
  * @brief Reads the given cmd arguments
  *
  * @param numberofargc argument count of the program
  * @param argv arguments of the program
  */
void getUserInputOrConfig(int numberofargc, char** argv);

/**
  * @brief Outputs a help message for the user
  */
void outputHelpMessage(void);

/**
  * @brief Checks all parameters for validity
  */
void checkParams(void);

/**
  * @brief Allocates the three time step arrays
  */
void initWaveConditions(void);

/**
  * @brief Simulates one time step with the wave equation
  * 
  * @param holdflag If not 0, indicates that the point at this x coordinate should be fixed ("hold")
  */
void simulateOneTimeStep(int holdflag);

/**
  * @brief Simulates a number of time steps
  * 
  * @return The elapsed time in seconds
  */
double simulateNumberOfTimeSteps(void);

/**
  * @brief Frees the memory from the time step arrays
  */
void finalizeWave(void);

/**
  * @brief Resets the time step arrays
  */
void resetWave(void);

/**
  * @brief Prints the new time step array values to console
  */
void outputNew(void);

/**
  * @brief Performs a benchmark
  */
void performBenchmark(void);

/**
  * @brief Returns the current state of the wave values
  *
  * @return A pointer to array of the current values
  */
double * getStep(void);

/**
  * @brief Returns the number of discrete points of the wave
  *
  * @return The number of discrete points of the wave
  */
int getNpoints(void);

/**
  * @brief Returns the number of time steps (can be 0 for loop)
  *
  * @return The number of time steps
  */
int getTpoints(void);

/**
  * @brief Returns the damping factor lambda
  *
  * @return The damping factor of the wave
  */
double getLambda(void);

/**
  * @brief Returns the current state of the showGui flag
  *
  * @return Whether to show the wave or not
  */
int useGui(void);

/**
  * @brief Returns the current state of the benchmark flag
  *
  * @return Whether to perform a benchmark or not
  */
int doBench(void);

#endif //__CORE_H_
