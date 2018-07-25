/**
 * Implements the main MPI program and visualisation to the user
 *
 * @file coreMPI.c
 * @author Chris Rebbelin s0548921
 * @date 2018-07-01
 * @brief This file contains the main MPI program
 */

#include "coreMPI.h"

#define err_buffer[MPI_MAX_ERROR_STRING]

#define CHECK(ierr)                                       \
{                                                         \
    int resultlen;                                        \
                                                          \
       if (ierr != MPI_SUCCESS) {                         \
            fprintf(stderr,"MPI_erroir\n");               \
            MPI_Error_string(ierr, err_buffer, &resultlen); \
            fprintf(stderr, err_buffer);                   \
            MPI_Finalize();                               \
            exit(EXIT_FAILURE);                           \
       }                                                  \
}                                                         \

const double PI = 3.14159265359; // gibts schon in math.h?

    int right;
    int left;

    int NPOINTS_GLOBAL;
    int NPOINTS_LOCAL;
    int TPOINTS;

    int periods;
    int amplitude;

    int useGui, printvalues;

    double DELTA_X, SPEED, lambda;
    const double DELTA_T = 1.0;

    int id;
    int numberOfProcesses;

    double *previousStep;
    double *currentStep;
    double *nextStep;

    double *u_global;

    int L_TO_R = 10;
    int R_TO_L = 20;

    int INFO = 30;
    int ACTUAL = 40;

    int buffer[2];

    MPI_Status status;

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
	printf("To perform benchmarks, use './myWave -b [-s SPEED] [-t TIMESTEPS] [-i INTERVALEND] [-n POINTS] [-p PERIODS] [-a AMPLITUDE] [-l LAMBDA]'\n");
	printf("To show this message, use './myWave -h' or './myWave --help'\n");
}

void getFromSettingsFile(char *configPath) {

    const int MAXLINE = 200;


    FILE * filePointer;
    char buffer[MAXLINE], configKey[50], configValue[50];
    int i, count;

    filePointer = fopen(configPath, "r");
    if(NULL == filePointer) {
        printf("[ERROR] Could not get file '%s'!\n", configPath);
    }

    while(NULL != fgets(buffer, MAXLINE, filePointer)) {
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

            outputHelpMessage();
            exit(EXIT_SUCCESS);

        } else if (0 == strcmp(argv[1], "-v") || 0 == strcmp(argv[1], "--version")) {

            printf("Wave equation MPI - Psys18\nChris Rebbelin 2018\nVersion 0.1\n");
            exit(EXIT_SUCCESS);

        } else if (0 == strcmp(argv[1], "-b") || 0 == strcmp(argv[1], "--benchmark")) {

            for (int i = 2; i < nargc; i++) {

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

                } else {

                    printf("Unrecognized argument: %s\n", argv[i]);
                    exit(EXIT_FAILURE);
                }

            }

            // disabled for benchmarks
            useGui = 0;
            printvalues = 0;

        } else {

            printf("Unrecognized argument: %s\n", argv[1]);
            exit(EXIT_FAILURE);
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
        printf("[ERROR] Dampening factor must not be negative!\n");
        exit(EXIT_FAILURE);
    }

    if (lambda > MAX_LAMBDA) {
        printf("[ERROR] Dampening factor must not be greater than %.2f!\n", MAX_LAMBDA);
        exit(EXIT_FAILURE);
    }

	if (!useGui && TPOINTS == 0) {
		printf("[ERROR] Simulating an endless loop is only allowed with visualisation enabled!\n");
		exit(EXIT_FAILURE);
	}

	DELTA_X = (L / NPOINTS);

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
    lambda = 0; //1 / (double) TPOINTS;
	useGui = 1;
    printvalues = 0;

    if (numberofargc > 1) {

        getFromCmdLine(numberofargc, argv);

    } else {

        // use default settings file
        getFromSettingsFile(DEFAULT_SETTINGS_FILE_PATH);
    }

    checkParams();
}

double waveInitFunc(double x) {
   	return (amplitude * sin(2*x*PI*periods/L));
}

void simulateOneTimeStep(int holdflag) {

        // calculate next time step with wave equation
        for (int i = 1; i < right - left; i++ ) {
            nextStep[i] = 2.0 * currentStep[i] - previousStep[i] + C_SQUARED * (currentStep[i - 1] - (2.0 * currentStep[i]) + currentStep[i + 1]);
        }

        if (id != MASTER) {
            // exchange border values with the left neighbor
            CHECK(MPI_Send(&nextStep[1], 1, MPI_DOUBLE, id - 1, R_TO_L, MPI_COMM_WORLD));
            CHECK(MPI_Recv(&nextStep[0], 1, MPI_DOUBLE, id - 1, L_TO_R, MPI_COMM_WORLD, &status));
        } else {
            // MASTER is the "leftmost" process and has no left neighbor but the boundary condition
            nextStep[0] = 0.0;
        }

        if (id != LAST) {
            // exchange border values with the right neighbor
            CHECK(MPI_Send(&nextStep[right - left - 1], 1, MPI_DOUBLE, id + 1, L_TO_R, MPI_COMM_WORLD));
            CHECK(MPI_Recv(&nextStep[right - left], 1, MPI_DOUBLE, id + 1, R_TO_L, MPI_COMM_WORLD, &status));
        } else {
            // last process is the "rightmost" and has no right neighbor but the boundary condition
            nextStep[right - left] = 0.0
        }

        // alle einen zeitschritt in die past kopieren
        for (int j = 0; j <= right - left; j++) {
            previousStep[j] = currentStep[j];
            currentStep[j] = nextStep[j];
        }
}

void simulateNumberOfTimeSteps() {

    // time steps
    for (int i = 1; i < TPOINTS; ++i) {
        simulateOneTimeStep(0);
    }
}

void outputNew() {

	printf("####Current Values:####\n");
    for (int l = 0; l < NPOINTS; ++l) {
        printf("%4d => %6.6f\n", l, u_global[l]);
    }
}

void initWaveConditions(int id, int numberOfProcesses) {

    const int MASTER = 0;
    const int LAST = numberOfProcesses - 1;

    // broadcast the settings to all processes
    CHECK(MPI_Bcast(&NPOINTS_GLOBAL, 1, MPI_INT, MASTER, MPI_COMM_WORLD));
    CHECK(MPI_Bcast(&TPOINTS, 1, MPI_INT, MASTER, MPI_COMM_WORLD));
    CHECK(MPI_Bcast(&C_SQUARED, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
    CHECK(MPI_Bcast(&DELTA_X, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD));

    // calculate left and right border for every process
    left = (   id       * ( NPOINTS_GLOBAL - 1 ) ) / numberOfProcesses;
    right = ( ( id + 1 ) * ( NPOINTS_GLOBAL - 1 ) ) / numberOfProcesses;

    if (id != MASTER) {
        left = left - 1;
    }
    NPOINTS_LOCAL = right + 1 - left;

    // allocate space for local arrays
    const size_t bufSize = NPOINTS_LOCAL * sizeof(double);
    previousStep = ( double * ) malloc(bufSize);
    currentStep = ( double * ) malloc(bufSize);
    nextStep = ( double * ) malloc(bufSize);

    // master needs another global array to collect everything in the end
    if (id == MASTER) {
        u_global = (double *) malloc(NPOINTS_GLOBAL * sizeof(double));
    }

    // initialize the first time step
    for (int k = 0; k <= right - left; k++ ) {
        x = (k + left) * DELTA_X;
        previousStep[k] = waveInitFunc(x);
        currentStep[k] = waveInitFunc(x);
    }
}

void finalizeWave(int id, int numberOfProcesses) {


    // if Master, collect all others
    if ( id == MASTER) {

        // write own results to global array first
        for ( i = 0; i <= right; i++ ) {
            u_global[i] = currentStep[i];
        }

        // recieve results from every other process and write it
        for (i = 1; i < numberOfProcesses; i++ ) {

            // man kann nich einfach so die werte schicken, weil master nich weiß
            // wieviele er recieven soll
            // also schickt man ihm erst die info, wieviele und wo die hin sollen
            CHECK(MPI_Recv(buffer, 2, MPI_INT, i, collect1, MPI_COMM_WORLD, &status));
            
            left = buffer[0]; // // startindex im globalen array
            count = buffer[1]; // wieviele werte werden gleich gesendet

            // erwarte genau <count> viele werte vom typ <double> von prozess <i>
            // diese werden in das globale array geschrieben, beginnend ab index <left>
            CHECK(MPI_Recv(&u_global[left], count, MPI_DOUBLE, i, collect2, MPI_COMM_WORLD, &status));
        }

        printf ("Elapsed wallclock time total was %g seconds\n", MPI_Wtime() - wtime);
        if (printvalues) {
          outputNew(); 
        }
        free (u_global);

    } else { // if not master, send to master

        buffer[0] = left; // startindex im globalen array
        buffer[1] = NPOINTS_LOCAL; // wieviele
        
        CHECK(MPI_Send(buffer, 2, MPI_INT, 0, INFO, MPI_COMM_WORLD)); // erst die vorinfos senden
        CHECK(MPI_Send(currentStep, NPOINTS_LOCAL, MPI_DOUBLE, 0, ACTUAL, MPI_COMM_WORLD)); // dann die eigentlichen werte 
    }

    free(currentStep);
    free(previousStep);
    free(nextStep);

    MPI_Finalize();
    return 0;
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
