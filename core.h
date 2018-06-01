#ifndef _CORE_H
#define _CORE_H

#include <stdlib.h>
#include<stdio.h>
#include <math.h>
#include<string.h>

void calc(int index);
void init(void);
void simulateOneTimeStep(void);
void simulateNumberOfTimeSteps(unsigned int numberOf);
void close(void);
void outputNew(void);


#endif
