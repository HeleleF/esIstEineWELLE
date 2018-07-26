/**
 * @file coreMPI.h
 * @author Chris Rebbelin s0548921
 * @date 2018-07-01
 * @brief contains the main MPI calculation logic for the wave
 */

#ifndef __CORE_MPI_H_
#define __CORE_MPI_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <omp.h>
# include "mpi.h"


void CHECK(int ierr);

/**
  * Calculates the initial sine wave values 
  *
  * @param x The x value
  * @return The value of the sine function at x
  */
double waveInitFunc(double x);

/**
  * Reads the settings from file specified
  * by a given file path
  *
  * @param configPath Path to a settings file
  */
void getFromSettingsFile(char *configPath);

/**
  * Parses the settings from command line
  *
  * @param nargc argument count of the program
  * @param argv arguments of the program
  */
void getFromCmdLine(int nargc, char** argv);

/**
  * Reads the given cmd arguments
  *
  * @param numberofargc argument count of the program
  * @param argv arguments of the program
  * @param pid ID of the mpi process
  * @param pnum number of mpi processes
  */
void getUserInputOrConfig(int numberofargc, char** argv, int pid, int pnum);

/**
  * Outputs a help message for the user
  */
void outputHelpMessage(void);

/**
  * Checks all parameters for validity
  */
void checkParams(void);

/**
  * Allocates the three time step arrays and 
  * initializes the first two time steps with the sine curve
  */
void initWaveConditions(void);

/**
  * Simulates one time step with the wave equation
  */
void simulateOneTimeStep(int holdflag);

/**
  * Calls simulateOneTimeStep() a specified number of times
  */
double simulateNumberOfTimeSteps(void);

/**
  * Frees the memory from the time step arrays
  */
void finalizeWave(void);

/**
  * Resets the time step arrays to the
  * inital sine wave.
  */
void resetWave(void);

/**
  * Prints the new time step array values to console
  */
void outputNew(void);

/**
  * Performs a benchmark
  */
void performBenchmark(void);

/**
  * collects calculated values from all processes 
  * into a global array in the master process
  */
void collectWave(void);

/**
  * Returns the current state of the wave values.
  *
  * @return A pointer to array of the current values
  */
double * getStep(void);

/**
  * Returns the number of discrete points of the wave.
  *
  * @return The number of discrete points of the wave
  */
int getNPOINTS(void);

/**
  * Returns the number of time steps (can be 0 for loop).
  *
  * @return The number of time steps
  */
int getTPOINTS(void);

/**
  * Returns the dampening factor lambda.
  *
  * @return The dampening factor of the wave
  */
double getLAMBDA(void);

/**
  * Returns the current state of the showGui flag.
  *
  * @return Whether to show the wave or not
  */
int useGUI(void);

/**
  * Returns the current state of the doBenchmark flag.
  *
  * @return Whether to perform a benchmark or not
  */
int doBench(void);

#endif
