/**
 * Implements the core of the algorithm.
 *
 * @file core.c
 */

#include "core.h"
#include "omp.h"
#include <math.h>

double *previousStep, *currentStep, *nextStep;

unsigned int npoints = 20; //number of points on the string (i)
unsigned int tpoints = 10; // number of time steps

void calc(int i) {
	double c = 0.3; // this is user input

	const double dt = 1.0;
	const double dx = 1.0;
	double tau, sqtau;

	tau = c * (dt / dx);
	sqtau = tau * tau;
	nextStep[i] = 2.0 * currentStep[i] - previousStep[i]
			+ (sqtau
					* (currentStep[i - 1] - (2.0 * currentStep[i])
							+ currentStep[i + 1]));
}

void init() {

	currentStep = malloc(npoints * sizeof(double));
	previousStep = malloc(npoints * sizeof(double));
	nextStep = malloc(npoints * sizeof(double));

	memset(previousStep, 0, npoints);
	memset(currentStep, 0, npoints);
	memset(nextStep, 0, npoints);

	for (int x = 1; x < npoints - 1; x++) {
		previousStep[x] = sin(x * 2.0);
		currentStep[x] = sin(x * 2.0);
	}
}

void simulateOneTimeStep() {

	/* update points along line */
	for (int j = 1; j < npoints - 1; j++) {
		calc(j);
	}

	nextStep[0] = 0.0;
	nextStep[npoints - 1] = 0.0;

	//performance- => many write operations to swap the arrays
	for (int k = 0; k < npoints; k++) {
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

void simulateNumberOfTimeSteps(unsigned int numberOfTimeSteps) {

	// time steps
	for (int i = 0; i < numberOfTimeSteps; ++i) {
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

