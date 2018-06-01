/**
 * Implements the core of the algorithm.
 *
 * @file core.c
 */

#include "core.h"
#include "omp.h"
#include <math.h>

const double PI = 3.141592653;

double *previousStep, *currentStep, *nextStep;

int NPOINTS; //number of points on the string (i)
int TPOINTS; // number of time steps

double DELTA_T; //delta t, dt
double DELTA_X; //delta x, dx

double L;
double T;

double C;
double COURANT;
double COURANT_SQUARED;

double (*I)(double);


double myInitFunc(double x) {
	return  (2 * sin(x*PI/L));
}

void getUserInputOrConfig() {

	C = 1.5;
	COURANT = 0.75 // counrant zahl, muss zwischen 0 und 1 sein
	COURANT_SQUARED = COURANT * COURANT; // das ist der faktor aus der gleichung

	T = 10 // von wo bis wo soll die zeit gehen -> von 0 bis T
	DELTA_T = 0.05 // das ist der zeitschritt abstand, 
	TPOINTS = (int) (T / DELTA_T); // in wie viele zeitpunkte wird die zeit eingeteilt

	L = 5; // von wo bis wo soll der string gehen -> von 0 bis L
	DELTA_X = DELTA_T * C / COURANT
	NPOINTS = (int) (L / DELTA_X); // in wie viele punkte wird der string unterteilt

	I = &myInitFunc; // initaer werte aus dieser funktion nehmen
}

void init() {

	double x;

	// arrays initialiserien und auf 0 setzen
	const int bufSize = (NPOINTS+1) * sizeof(double);
	previousStep = malloc(bufSize);
	currentStep = malloc(bufSize);
	nextStep = malloc(bufSize);

	memset(previousStep, 0, NPOINTS);
	memset(currentStep, 0, NPOINTS);
	memset(nextStep, 0, NPOINTS);

	for (int i = 0; i < NPOINTS + 1; i++) {

		x = i * dx;
		previousStep[i] = I(x);
	}

	for (int i = 1; i < NPOINTS; i++) {

		currentStep[i] = previousStep[i] + 0.5 * COURANT_SQUARED * (previousStep[i-1] - (2.0 * previousStep[i]) + previousStep[i+1]);
	}
}

void simulateOneTimeStep() {

	/* update points along line */
	for (int i = 1; i < NPOINTS; i++) {
			nextStep[i] = 2.0 * currentStep[i] - previousStep[i] + COURANT_SQUARED * (currentStep[i - 1] - (2.0 * currentStep[i]) + currentStep[i + 1]);
	}

	nextStep[0] = 0.0;
	nextStep[NPOINTS] = 0.0;

	//performance- => many write operations to swap the arrays
	for (int k = 0; k < NPOINTS+1; k++) {
		previousStep[k] = currentStep[k];
		currentStep[k] = nextStep[k];
	}

	outputNew();

	/* //performance+ => use array swap directly via pointer
	 double *tempStep = previousStep;
	 previousStep = currentStep;
	 currentStep = nextStep;
	 nextStep = tempStep;*/

}

void simulateNumberOfTimeSteps() {

	// time steps
	for (int i = 1; i < TPOINTS; ++i) {
		simulateOneTimeStep();
	}

}

void close() {

	free(previousStep);
	free(currentStep);
	free(nextStep);

}

void outputNew() {

	for (int l = 0; l < npoints; ++l) {
		printf("%f\n", nextStep[l]);
	}
	printf("-------------------------------------------\n");
}

