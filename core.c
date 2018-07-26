/**
 * Implements the main calculation logic for the wave
 *
 * @file core.c
 * @author Chris Rebbelin s0548921
 * @date 2018-07-01
 * @brief This file contains the main calculation logic for the wave
 */

#include <omp.h>
#include "core.h"

#define MAX_POINTS 10000000

#define MAX_LAMBDA 0.1

#define DEFAULT_SETTINGS_FILE_PATH "settings.txt"

#define BENCHMARK_FILE "benchResults.txt"


// time step arrays
double *previousStep, *currentStep, *nextStep;

// setting values
int L, NPOINTS, TPOINTS, periods, amplitude, useGui, printvalues, doBenchmark;

const double DELTA_T = 1.0;
double DELTA_X, lambda, C, C_SQUARED, SPEED;


double waveInitFunc(double x) {
   	return (amplitude * sin(2 * x * M_PI * periods / L));
}

void outputHelpMessage() {
    printf("\nHOW TO USE:\n");
	printf("Edit parameters in %s\n", DEFAULT_SETTINGS_FILE_PATH);
	printf("or use your own settings file with './myWave -c <pathToYourFile>'\n");
	printf("Available settings\n");
	printf("\tKEY\t\t\tDESCRIPTION\t\t\t\t\t\t\tDEFAULT\n\n");
	printf("\tSPEED\t\t\tThe speed of the wave, has to be between 0 and 1\t\t0.9\n");
	printf("\tNUMBER_OF_TIME_STEPS\tHow long the simulation should run, use 0 for infinite loop\t1000\n");
	printf("\tLINE_INTERVAL_END\tThe right end of the x-axis interval (left end is 0)\t\t1000\n");
	printf("\tNUMBER_OF_POINTS\tThe number of discret points on the line\t\t\t1000\n");
	printf("\tNUMBER_OF_PERIODS\tNumber of periods for the sine wave\t\t\t\t5\n");
	printf("\tAMPLITUDE\t\tThe amplitude of the sine wave\t\t\t\t\tLINE_INTERVAL_END / 4\n");
    printf("\tLAMBDA\t\t\tThe damping factor for the sine wave\t\t\t\t0 (no damping)\n");
	printf("\tSHOW_GUI\t\tShow the visualisation of the wave\t\t\t\t1 (true)\n");
	printf("\tPRINT_VALUES\t\tPrint final values to console\t\t\t\t\t0 (false)\n\n");
	printf("From cmd line: './myWave [-s SPEED] [-t TIMESTEPS] [-i INTERVALEND] [-n POINTS] [-p PERIODS] [-a AMPLITUDE] [-l LAMBDA] [-u SHOWGUI] [-v PRINTVALUES]'\n\n");
    printf("To perform benchmarks, use './myWave -b TIMESTEPS POINTS' or './myWave --benchmark TIMESTEPS POINTS'\n");
	printf("To show this message, use './myWave -h' or './myWave --help'\n");
}

void getFromSettingsFile(char *configPath) {

    const int MAXLINE = 200;

    FILE * filePointer;
    char buffer[MAXLINE], configKey[50], configValue[50];
    int i, count;

    // try to open the file
    filePointer = fopen(configPath, "r");
    if (NULL == filePointer) {
        printf("[ERROR] Could not get file '%s'!\n", configPath);
        exit(EXIT_FAILURE);
    }

    // read the contents
    while (NULL != fgets(buffer, MAXLINE, filePointer)) {
        count = 0;
        i = 0;

        // ignore comments or empty lines
        if ('#' == buffer[0] || '\n' == buffer[0]) {
            continue;
        }

        // get the key
        while (' ' != buffer[i]) {
            configKey[i] = buffer[i];
            i++;
        }
        configKey[i++]='\0';
        count=i;

        // get corresponding value
        while ('\n' != buffer[i]) {
            configValue[i-count] = buffer[i];
            i++;
        }
        configValue[i-count] = '\0';

        // set the corresponding setting
        if (0 == strcmp(configKey, "SPEED")) {
            SPEED = atof(configValue);
        } else if (0 == strcmp(configKey, "NUMBER_OF_TIME_STEPS")) {
            TPOINTS = atoi(configValue);
        } else if (0 == strcmp(configKey, "LINE_INTERVAL_END")) {
            L = atoi(configValue);
        } else if (0 == strcmp(configKey, "NUMBER_OF_POINTS")) {
            NPOINTS = atoi(configValue);
        } else if (0 == strcmp(configKey, "SHOW_GUI")) {
            useGui = atoi(configValue);
        } else if (0 == strcmp(configKey, "NUMBER_OF_PERIODS")) {
            periods = atoi(configValue);
        } else if (0 == strcmp(configKey, "AMPLITUDE")) {
            amplitude = atoi(configValue);
        } else if (0 == strcmp(configKey, "LAMBDA")) {
            lambda = atof(configValue);
        } else if (0 == strcmp(configKey, "PRINT_VALUES")) {
            printvalues = atoi(configValue);
        } else {
        	printf("[WARNING] Unrecognized settings key: '%s'\n", configKey);
        }
    }
}

void getFromCmdLine(int nargc, char** argv) {

        // check cmdline arguments
        if (0 == strcmp(argv[1], "-c") || 0 == strcmp(argv[1], "--use-config-file")) {

            // use custom settings file
            getFromSettingsFile(argv[2]);

        } else if (0 == strcmp(argv[1], "-h") || 0 == strcmp(argv[1], "--help")) {

            // print help message and exit
            outputHelpMessage();
            exit(EXIT_SUCCESS);

        } else if (0 == strcmp(argv[1], "-v") || 0 == strcmp(argv[1], "--version")) {

            // print version and exit
            printf("Wave equation - Psys18\nChris Rebbelin 2018\nVersion 0.1\n");
            exit(EXIT_SUCCESS);

        } else if (0 == strcmp(argv[1], "-b") || 0 == strcmp(argv[1], "--benchmark")) {

            if (nargc == 4) {

                doBenchmark = 1;

                // only number of timesteps and points can be set in benchmarks
                // all other values are default
                TPOINTS = atoi(argv[2]);
                NPOINTS = atoi(argv[3]);

                // visualization is disabled
                useGui = 0;

            } else {

                // print help message and exit
                outputHelpMessage();
                exit(EXIT_FAILURE);
            }

        } else {

            // iterate over all argv
            for (int i = 1; i < nargc; i++) {

                if (0 == strcmp(argv[i], "-s") || 0 == strcmp(argv[i], "--speed")) {

                    SPEED = atof(argv[++i]);

                } else if (0 == strcmp(argv[i], "-t") || 0 == strcmp(argv[i], "--timesteps")) {

                    TPOINTS = atoi(argv[++i]);

                } else if (0 == strcmp(argv[i], "-i") || 0 == strcmp(argv[i], "--intervalend")) {

                    L = atoi(argv[++i]);

                } else if (0 == strcmp(argv[i], "-n") || 0 == strcmp(argv[i], "--npoints")) {

                    NPOINTS = atoi(argv[++i]);

                } else if (0 == strcmp(argv[i], "-p") || 0 == strcmp(argv[i], "--periods")) {

                    periods = atof(argv[++i]);

                } else if (0 == strcmp(argv[i], "-a") || 0 == strcmp(argv[i], "--amplitude")) {

                    amplitude = atof(argv[++i]);

                } else if (0 == strcmp(argv[i], "-l") || 0 == strcmp(argv[i], "--lambda")) {

                    lambda = atof(argv[++i]);

                } else if (0 == strcmp(argv[i], "-u") || 0 == strcmp(argv[i], "--usegui")) {

                    useGui = atoi(argv[++i]);

                } else if (0 == strcmp(argv[i], "-v") || 0 == strcmp(argv[i], "--printvalues")) {

                    printvalues = atoi(argv[++i]);

                } else {

                    printf("Unrecognized argument: %s\n", argv[i]);
                }
            }
        }
}

void checkParams() {

	if (SPEED <= 0 || SPEED >= 1) {
		printf("[ERROR] Wave equation not stable with C=%.3f!\n", SPEED);
		exit(EXIT_FAILURE);
	}

	if (TPOINTS < 0) {
		printf("[ERROR] Number of time steps must not be negative!\n");
		exit(EXIT_FAILURE);
	}

	if (L < 1) {
		printf("[ERROR] Right interval border must not be smaller than 1\n");
		exit(EXIT_FAILURE);
	}

	if (NPOINTS <= 0) {
		printf("[ERROR] Number of discrete points must not be negative or zero!\n");
		exit(EXIT_FAILURE);
	}

	if (NPOINTS > MAX_POINTS) {
		printf("[ERROR] Number of discrete points is bigger than the allowed maximum of %d!\n", MAX_POINTS);
		exit(EXIT_FAILURE);
	}

	if (periods < 1) {
		printf("[ERROR] Period length must not be smaller than 1!\n");
		exit(EXIT_FAILURE);
	}

	if (amplitude < 1) {
		printf("[ERROR] Amplitude must not be smaller than 1!\n");
		exit(EXIT_FAILURE);
	}

    if (lambda < 0) {
        printf("[ERROR] Damping factor must not be negative!\n");
        exit(EXIT_FAILURE);
    }

    if (lambda > MAX_LAMBDA) {
        printf("[ERROR] Damping factor must not be greater than %.2f!\n", MAX_LAMBDA);
        exit(EXIT_FAILURE);
    }

	if (!useGui && TPOINTS == 0) {
		printf("[ERROR] Simulating an endless loop is only allowed with visualisation enabled!\n");
		exit(EXIT_FAILURE);
	}

    // calculate delta x
	DELTA_X = (L / NPOINTS);

    // calculate c^2
	C = (DELTA_T / DELTA_X) * SPEED;
	C_SQUARED = C * C;

    if (TPOINTS == 0) {
    	printf("Looping forever"); 
    } else {
     	printf("Simulating %d time steps", TPOINTS);
    }
	printf(" with parameters:\n"); 
    printf("Using %d discrete points in line interval [0,%d] with speed %.3f\n", NPOINTS, L, SPEED);
    printf("Simulating a %sdampened", lambda == 0 ? "un" : "");
    printf(" sine wave with amplitude %d and %d periods", amplitude, periods);
    if (lambda != 0) printf(" and damping factor %f", lambda);
    printf("\n");
}

void getUserInputOrConfig(int numberofargc, char** argv) {

	// default values
	SPEED = 0.9;
	TPOINTS = 1000;
	L = 1000;
	NPOINTS = 1000;
	periods = 5;
	amplitude = L / 4;
    lambda = 0;
	useGui = 1;
    printvalues = 0;
    doBenchmark = 0;

    if (numberofargc > 1) {

        getFromCmdLine(numberofargc, argv);

    } else {

        // use default settings file
        getFromSettingsFile(DEFAULT_SETTINGS_FILE_PATH);
    }

    checkParams();
}

void initWaveConditions() {

    // initialize arrays
    const size_t bufSize = (NPOINTS+1) * sizeof(double);
    previousStep = malloc(bufSize);
    currentStep = malloc(bufSize);
    nextStep = malloc(bufSize);

    resetWave();
}

void simulateOneTimeStep(int holdflag) {

    int i;

    #pragma omp parallel for shared(nextStep, currentStep, previousStep, C_SQUARED, NPOINTS, holdflag) private(i)
    for (i = 1; i < NPOINTS; i++) {

        if (holdflag == i) {
            // Point at holdflag is fixed, so don't calculate a new position for it, just use the old one
            nextStep[holdflag] = currentStep[holdflag];
        } else {
            nextStep[i] = 2.0 * currentStep[i] - previousStep[i] + C_SQUARED * (currentStep[i - 1] - (2.0 * currentStep[i]) + currentStep[i + 1]);
        }   
    }

	// update boundary conditions
    nextStep[0] = 0.0;
    nextStep[NPOINTS] = 0.0;

    // copy values one step "into the past"
    double *tempStep = previousStep;
    previousStep = currentStep;
    currentStep = nextStep;
    nextStep = tempStep;
}

double simulateNumberOfTimeSteps() {

    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start);

    for (int i = 1; i < TPOINTS; ++i) {
        simulateOneTimeStep(0);
    }

    clock_gettime(CLOCK_REALTIME, &end);

    if (printvalues) {
    	outputNew();
    }

    return ((end.tv_nsec - start.tv_nsec) / 1E9 + (end.tv_sec - start.tv_sec));
}

void finalizeWave() {

    free(previousStep);
    free(currentStep);
    free(nextStep);
}

void resetWave() {

    double x;
    int i;

    memset(previousStep, 0, NPOINTS+1);
    memset(currentStep, 0, NPOINTS+1);
    memset(nextStep, 0, NPOINTS+1);

    for (i = 0; i < NPOINTS + 1; i++) {

        x = i * DELTA_X;
        previousStep[i] = waveInitFunc(x);
        currentStep[i] = waveInitFunc(x);
    }
}

void outputNew() {

	printf("####Current Values:####\n");
    for (int l = 0; l < NPOINTS + 1; ++l) {
        printf("%4d => %6.6f\n", l, currentStep[l]);
    }
}

void performBenchmark() {

    const int RERUNS = 10;
    double runtime[RERUNS]; 

    memset(runtime, 0, RERUNS);

    double mean = 0.0;
    double stddev = 0.0;

    initWaveConditions();

    // run repeatedly
    for (int i = 0; i < RERUNS; i++) {
        runtime[i] = simulateNumberOfTimeSteps();
        resetWave();
    }

    finalizeWave();

    // calculate run time statistics
    for (int i = 0; i < RERUNS; i++) {
        mean += runtime[i];
    }
    mean = mean / RERUNS;

    for (int i = 0; i < RERUNS; i++) {
        stddev += pow(runtime[i] - mean, 2);
    }
    stddev = sqrt(stddev / RERUNS);

    printf("%lf %lf\n", mean, stddev);

    FILE *fp;
    fp = fopen(BENCHMARK_FILE, "a");

    if (NULL == fp) {
        printf("[ERROR] Could not get file '%s'!\n", BENCHMARK_FILE);
        exit(EXIT_FAILURE);
    }
 
    fprintf(fp, "Running for %5d timesteps with %10d points took %5.5f seconds with stddev = %5.5f after %d reruns.\n", TPOINTS, NPOINTS, mean, stddev, RERUNS);
    fclose(fp);
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

double getLAMBDA() {
    return lambda;
}

int useGUI() {
    return useGui;
}

int doBench() {
    return doBenchmark;
}
