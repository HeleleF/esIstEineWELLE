/**
 * @file coreMPI.h
 * @author Chris Rebbelin s0548921
 * @date 2018-07-29
 * @brief header file for @c coreMPI.c
 * 
 * @details This file contains all needed definitions and includes
 * for the main mpi calculation logic implemented in @c coreMPI.c.
 */

#ifndef __CORE_MPI_H_
#define __CORE_MPI_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <mpi.h>

/**
 * @def _USE_MATH_DEFINES
 * 
 * Used for the constant @c M_PI from @c math.h.
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
#define DEFAULT_SETTINGS_FILE_PATH "../settings.txt"

/**
 * @def BENCHMARK_FILE
 * 
 * The default output file used for benchmark results
 */
#define BENCHMARK_FILE "../benchmark/benchResults.txt"


/**
  * @brief Checks a given MPI return code for errors
  *
  * @param ierr The MPI return code
  */
void CHECK(int ierr);

/**
  * @brief Outputs a help message for the user
  */
void outputHelpMessage(void);

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
  * @brief Checks all parameters for validity
  */
void checkParams(void);

/**
  * @brief Reads the given cmd arguments
  *
  * @param numberofargc argument count of the program
  * @param argv arguments of the program
  * @param pid ID of the mpi process
  * @param pnum number of mpi processes
  */
void getUserInputOrConfig(int numberofargc, char** argv, int pid, int pnum);

/**
  * @brief Calculates the initial sine wave values 
  *
  * @param x The x value
  * @return The value of the sine function at @c x
  */
double waveInitFunc(double x);

/**
  * @brief Allocates the three time step arrays
  */
void initWaveConditions(void);

/**
  * @brief Simulates one time step with the wave equation
  */
void simulateOneTimeStep(void);

/**
  * @brief Calls @c simulateOneTimeStep() a specified number of times
  */
double simulateNumberOfTimeSteps(void);

/**
  * @brief Collects calculated values from all processes 
  */
void collectWave(void);

/**
  * Frees the memory from the time step arrays
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
  * @brief Returns the number of time steps
  * 
  * (can be 0 for loop)
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
int showGui(void);

/**
  * @brief Returns the current state of the benchmark flag
  *
  * @return Whether to perform a benchmark or not
  */
int doBench(void);

#endif //__CORE_MPI_H_
