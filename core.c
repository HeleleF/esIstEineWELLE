/**
 * Implements the core of the algorithm.
 *
 * @file core.c
 */

#include "core.h"
#include <omp.h>
#include <math.h>

//TODO: alles rivhtig bennen und an die richtigen stellen packen cconvention stil
//TODO: auch den malloc usw timen und erwähnen vllt
//TODO: schonmal über benchmark script gedanken machen

const double PI = 3.141592653;

double *previousStep, *currentStep, *nextStep;

int useGui;

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
	return  (25 * sin(x*PI/(L/10)));
}

void getFromSettingsFile(char *configPath) {

    const int MAXLINE = 100;


    FILE * filePointer;
    char buffer[MAXLINE], configKey[20], configValue[10];
    int i, count;

    filePointer = fopen(configPath,"r");
    if(NULL == filePointer)
    {
        printf("Error getting config file '%s'!\n", configPath);
    } 

    while(NULL != fgets(buffer, MAXLINE, filePointer))
    {
        count = 0;
        i = 0;

        if ('#' == buffer[0]) //ignore comments
        {
            continue;
        }
        
        while (' ' != buffer[i]) //get config key
        {
            configKey[i] = buffer[i];
            i++;
        }
        configKey[i++]='\0';
        count=i;

        while ('\n' != buffer[i]) //get config value
        {
            configValue[i-count] = buffer[i];
            i++;
        }
        configValue[i-count] = '\0';

        if (0 == strcmp(configKey, "C"))
        {
            C = atof(configValue); 
        } 
        else if (0 == strcmp(configKey, "COURANT_NUMBER"))
        {
            COURANT = atof(configValue); // counrant zahl, muss zwischen 0 und 1 sein
            COURANT_SQUARED = COURANT * COURANT; // das ist der faktor aus der gleichung
        } 
        else if (0 == strcmp(configKey, "TIME_INTERVAL_END"))
        {
            T = atof(configValue); // von wo bis wo soll die zeit gehen -> von 0 bis T
        } 
        else if (0 == strcmp(configKey, "TIME_STEP"))
        {
            DELTA_T = atof(configValue); // das ist der zeitschritt abstand, 
        }
        else if (0 == strcmp(configKey, "LINE_INTERVAL_END"))
        {
            L = atof(configValue); // von wo bis wo soll der string gehen -> von 0 bis L
        }
        else if (0 == strcmp(configKey, "SHOW_GUI"))
        {
            useGui = atoi(configValue); // use gui
        }
    }

    TPOINTS = (int) (T / DELTA_T); // in wie viele zeitpunkte wird die zeit eingeteilt

	DELTA_X = DELTA_T * C / COURANT;
	NPOINTS = (int) (L / DELTA_X); // in wie viele punkte wird der string unterteilt

	I = &myInitFunc; // initaer werte aus dieser funktion nehmen
}


void getUserInputOrConfig(int numberofargc, char** argv) {

	if (numberofargc > 1) {

		// es gibt cmd line args
		if (0 == strcmp(argv[1], "-c") || 0 == strcmp(argv[1], "--use-config-file")) {
			getFromSettingsFile(argv[2]);
		} else {
			// manually parse args
		}
	} else {

		// keine cmd args, also standard setting file nehmen
		getFromSettingsFile("settings.txt");
	}
	printf("Using C = %f and Courant number = %f in a Time interval [0 , %f] with time step width of %f over line interval [0 , %f]\n", C,COURANT,T,DELTA_T,L);
	printf("Total number of time points = %d, DELTA_X =  dx %f and total number of line points = %d\n", TPOINTS,DELTA_X,NPOINTS);

}

void initMeins() {

	double x;
	int i;

	// arrays initialiserien und auf 0 setzen
	const size_t bufSize = (NPOINTS+1) * sizeof(double);
	previousStep = malloc(bufSize);
	currentStep = malloc(bufSize);
	nextStep = malloc(bufSize);

	memset(previousStep, 0, NPOINTS);
	memset(currentStep, 0, NPOINTS);
	memset(nextStep, 0, NPOINTS);

	for (i = 0; i < NPOINTS + 1; i++) {

		x = i * DELTA_X;
		previousStep[i] = I(x);
	}

	for (i = 1; i < NPOINTS; i++) {

		currentStep[i] = previousStep[i] + 0.5 * COURANT_SQUARED * (previousStep[i-1] - (2.0 * previousStep[i]) + previousStep[i+1]);
	}
}

void simulateOneTimeStep() {

	int i;

	
	

	/* update points along line */
	#pragma omp parallel for shared(nextStep, currentStep, previousStep, COURANT_SQUARED, NPOINTS) private(i)
	for (i = 1; i < NPOINTS; i++) {
			nextStep[i] = 2.0 * currentStep[i] - previousStep[i] + COURANT_SQUARED * (currentStep[i - 1] - (2.0 * currentStep[i]) + currentStep[i + 1]);
	}



	nextStep[0] = 0.0;
	nextStep[NPOINTS] = 0.0;

	/*//performance- => many write operations to swap the arrays
	for (int k = 0; k < NPOINTS+1; k++) {
		previousStep[k] = currentStep[k];
		currentStep[k] = nextStep[k];
	}*/

	//outputNew();

	 //performance+ => use array swap directly via pointer
	// with omp and 4 cores performance speedup of ~2.3 with this over the loop swap
	 double *tempStep = previousStep;
	 previousStep = currentStep;
	 currentStep = nextStep;
	 nextStep = tempStep;

}

void simulateNumberOfTimeSteps() {

	struct timeval astart, aend;
	gettimeofday(&astart, NULL);
	printf("start\n");
	// TODO: welche timer soll man nehmen?
	double start = omp_get_wtime();


	// time steps
	for (int i = 1; i < TPOINTS; ++i) {
		simulateOneTimeStep();
	}

	gettimeofday(&aend, NULL);
	printf("end\n");
	double end = omp_get_wtime();

	double elapsed = end - start;
	double adelta = ((aend.tv_sec - astart.tv_sec) * 1000000u + aend.tv_sec - astart.tv_sec) / 1e6;

	printf("Time ges:%f\n", adelta);
	printf("Time davon openmp:%f\n", elapsed);



}

void closeMeins() {

	free(previousStep);
	free(currentStep);
	free(nextStep);

}

void outputNew() {

	for (int l = 0; l < NPOINTS+1; ++l) {
		printf("%f\n", nextStep[l]);
	}
	printf("-------------------------------------------\n");
}

double * getStep() {
	return currentStep;
}

int getNPOINTS() {
	return NPOINTS;
}

int getTPOINTS() {
	return TPOINTS;
}

int useGUI() {
	return useGui;
}
