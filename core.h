#ifndef _CORE_H
#define _CORE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

double myInitFunc(double x);
void getFromSettingsFile(char *configPath);
void getUserInputOrConfig(int argc,char** argv);

void initMeins(void);

void simulateOneTimeStep(void);
void simulateNumberOfTimeSteps(void);

void closeMeins(void);

void outputNew(void);

double * getStep(void);
int getNPOINTS(void);
int getTPOINTS(void);
int useGUI(void);

#endif
