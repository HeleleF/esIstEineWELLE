#ifndef _CORE_H
#define _CORE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

void getUserInputOrConfig(int argc, char** argv);

void init(void);

void simulateOneTimeStep(void);
void simulateNumberOfTimeSteps();

void close(void);

void outputNew(void);

#endif
