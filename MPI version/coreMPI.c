/**
 * Implements the main MPI program and visualisation to the user
 *
 * @file coreMPI.c
 * @author Chris Rebbelin s0548921
 * @date 2018-07-01
 * @brief This file contains the main MPI program
 */

#include "coreMPI.h"                                                          

#define MAX_POINTS 10000000

#define MAX_LAMBDA 0.1

#define DEFAULT_SETTINGS_FILE_PATH "settings.txt"     

#define BENCHMARK_FILE "benchResultsMPI.txt"                                                   

int right;
int left, cnt;

int NPOINTS_GLOBAL;
int NPOINTS_LOCAL;
int TPOINTS, L, C, C_SQUARED;

int periods;
int amplitude;

int doBenchmark;
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

const int MASTER = 0;
int LAST;

MPI_Status status;

void CHECK(int ierr) {                                                           
       if (ierr != MPI_SUCCESS) {                           
            fprintf(stderr,"MPI_error\n");                  
                                                            
            int resultlen;                                  
            char err_buffer[MPI_MAX_ERROR_STRING];          
                                                            
            MPI_Error_string(ierr, err_buffer, &resultlen); 
            fprintf(stderr, err_buffer);                    
                                                            
            MPI_Finalize();                                 
            exit(EXIT_FAILURE);                             
       }                                                    
} 

void outputHelpMessage() {
    printf("\nHOW TO USE:\n");
	printf("Edit parameters in %s\n", DEFAULT_SETTINGS_FILE_PATH);
	printf("or use your own settings file with 'mpiexec myWave -c <pathToYourFile>'\n");
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
    printf("From cmd line: 'mpiexec myWave [-s SPEED] [-t TIMESTEPS] [-i INTERVALEND] [-n POINTS] [-p PERIODS] [-a AMPLITUDE] [-l LAMBDA] [-u SHOWGUI] [-v PRINTVALUES]'\n\n");
    printf("To perform benchmarks, use 'mpiexec myWave -b TIMESTEPS POINTS' or 'mpiexec myWave --benchmark TIMESTEPS POINTS'\n");
	printf("To show this message, use 'mpiexec myWave -h' or 'mpiexec myWave --help'\n");
}

void getFromSettingsFile(char *configPath) {

    const int MAXLINE = 200;

    FILE * filePointer;
    char buffer[MAXLINE], configKey[50], configValue[50];
    int i, count;

    filePointer = fopen(configPath, "r");

    if (NULL == filePointer) {
        if (id == MASTER) {
            printf("[ERROR] Could not get file '%s'!\n", configPath);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

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

        if (0 == strcmp(configKey, "SPEED")) {
            SPEED = atof(configValue);
        } else if (0 == strcmp(configKey, "NUMBER_OF_TIME_STEPS")) {
            TPOINTS = atoi(configValue);
        } else if (0 == strcmp(configKey, "LINE_INTERVAL_END")) {
            L = atoi(configValue);
        } else if (0 == strcmp(configKey, "NUMBER_OF_POINTS")) {
            NPOINTS_GLOBAL = atoi(configValue);
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
        	if (id == MASTER) {
                printf("[WARNING] Unrecognized settings key: '%s'\n", configKey);
            }
        }
    }
}

void getFromCmdLine(int nargc, char** argv) {

        // check cmdline arguments
        if (0 == strcmp(argv[1], "-c") || 0 == strcmp(argv[1], "--use-config-file")) {

            // use custom settings file
            getFromSettingsFile(argv[2]);

        } else if (0 == strcmp(argv[1], "-h") || 0 == strcmp(argv[1], "--help")) {

            if (id == MASTER) {
                outputHelpMessage();
            }
            MPI_Finalize();
            exit(EXIT_SUCCESS);

        } else if (0 == strcmp(argv[1], "-v") || 0 == strcmp(argv[1], "--version")) {

            if (id == MASTER) {
                printf("Wave equation MPI - Psys18\nChris Rebbelin 2018\nVersion 0.1\n");
            }
            MPI_Finalize();
            exit(EXIT_SUCCESS);

        } else if (0 == strcmp(argv[1], "-b") || 0 == strcmp(argv[1], "--benchmark")) {

            if (nargc == 4) {

                doBenchmark = 1;

                // only number of timesteps and points can be set in benchmarks
                // all other values are default
                TPOINTS = atoi(argv[2]);
                NPOINTS_GLOBAL = atoi(argv[3]);

                // visualization is disabled
                useGui = 0;

            } else {

                // print help message and exit
                if (id == MASTER) {
                    outputHelpMessage();
                }
                MPI_Finalize();
                exit(EXIT_FAILURE);
            }

        } else {

            for (int i = 1; i < nargc; i++) {

                if (0 == strcmp(argv[i], "-s") || 0 == strcmp(argv[i], "--speed")) {

                    SPEED = atof(argv[++i]);

                } else if (0 == strcmp(argv[i], "-t") || 0 == strcmp(argv[i], "--timesteps")) {

                    TPOINTS = atoi(argv[++i]);

                } else if (0 == strcmp(argv[i], "-i") || 0 == strcmp(argv[i], "--intervalend")) {

                    L = atoi(argv[++i]);

                } else if (0 == strcmp(argv[i], "-n") || 0 == strcmp(argv[i], "--npoints")) {

                    NPOINTS_GLOBAL = atoi(argv[++i]);

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

                    if (id == MASTER) {
                        printf("Unrecognized argument: %s\n", argv[i]);
                    }
                }

            }
        }
}

void checkParams() {

	if (SPEED <= 0 || SPEED >= 1) {
		if (id == MASTER) {
            printf("[ERROR] Wave equation not stable with C=%.3f!\n", SPEED);
        }
        MPI_Finalize();
		exit(EXIT_FAILURE);
	}

	if (TPOINTS < 0) {
		if (id == MASTER) {
            printf("[ERROR] Number of time steps must not be negative!\n");
        }
        MPI_Finalize();
		exit(EXIT_FAILURE);
	}

	if (L < 1) {
		if (id == MASTER) {
            printf("[ERROR] Right interval border must not be smaller than 1\n");
        }
        MPI_Finalize();
		exit(EXIT_FAILURE);
	}

	if (NPOINTS_GLOBAL <= 0) {
		if (id == MASTER) {
            printf("[ERROR] Number of discrete points must not be negative or zero!\n");
        }
        MPI_Finalize();
		exit(EXIT_FAILURE);
	}

	if (NPOINTS_GLOBAL > MAX_POINTS) {
		if (id == MASTER) {
            printf("[ERROR] Number of discrete points is bigger than the allowed maximum of %d!\n", MAX_POINTS);
        }
        MPI_Finalize();
		exit(EXIT_FAILURE);
	}

	if (periods < 1) {
		if (id == MASTER) {
            printf("[ERROR] Period length must not be smaller than 1!\n");
        }
        MPI_Finalize();
		exit(EXIT_FAILURE);
	}

	if (amplitude < 1) {
		if (id == MASTER) {
            printf("[ERROR] Amplitude must not be smaller than 1!\n");
        }
        MPI_Finalize();
		exit(EXIT_FAILURE);
	}

    if (lambda < 0) {
        if (id == MASTER) {
            printf("[ERROR] Dampening factor must not be negative!\n");
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (lambda > MAX_LAMBDA) {
        if (id == MASTER) {
            printf("[ERROR] Dampening factor must not be greater than %.2f!\n", MAX_LAMBDA);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

	if (!useGui && TPOINTS == 0) {
		if (id == MASTER) {
            printf("[ERROR] Simulating an endless loop is only allowed with visualisation enabled!\n");
        }
        MPI_Finalize();
		exit(EXIT_FAILURE);
	}

	DELTA_X = (L / NPOINTS_GLOBAL);

	C = (DELTA_T / DELTA_X) * SPEED;
	C_SQUARED = C * C;

    if (id == MASTER) {

        if (TPOINTS == 0) {
    	   printf("Looping forever"); 
        } else {
     	  printf("Simulating %d time steps", TPOINTS);
        }
	    printf(" with parameters:\n"); 
        printf("Using %d discrete points in line interval [0,%d] with speed %.3f\n", NPOINTS_GLOBAL, L, SPEED);
        printf("Simulating a %sdampened", lambda == 0 ? "un" : "");
        printf(" sine wave with amplitude %d and %d periods", amplitude, periods);
        if (lambda != 0) printf(" and damping factor %f", lambda);
        printf("\n");
    }
}

void getUserInputOrConfig(int numberofargc, char** argv, int pid, int pnum) {

    id = pid;
    numberOfProcesses = pnum;

    LAST = pnum - 1;

	// default values
	SPEED = 0.9;
	TPOINTS = 1000;
	L = 1000;
	NPOINTS_GLOBAL = 1000;
	periods = 5;
	amplitude = L / 4;
    lambda = 0;
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
   	return (amplitude * sin(2 * x * M_PI * periods / L));
}

void simulateOneTimeStep(int holdflag) {

        int i;

        // calculate next time step with wave equation
        #pragma omp parallel for shared(nextStep, currentStep, previousStep, C_SQUARED, right, left) private(i)
        for (i = 1; i < right - left; i++ ) {
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
            nextStep[right - left] = 0.0;
        }

        // copy values one step "into the past"
        double *tempStep = previousStep;
        previousStep = currentStep;
        currentStep = nextStep;
        nextStep = tempStep;
}

double simulateNumberOfTimeSteps() {

    double elapsed, wtime = MPI_Wtime();

    // time steps
    for (int i = 1; i < TPOINTS; ++i) {
        simulateOneTimeStep(0);
    }

    collectWave();

    elapsed = MPI_Wtime() - wtime;

    if (id == MASTER && printvalues) {
        outputNew();
    }

    return elapsed;
}

void outputNew() {

	printf("####Current Values:####\n");
    for (int l = 0; l < NPOINTS_GLOBAL; ++l) {
        printf("%4d => %6.6f\n", l, u_global[l]);
    }
}

void resetWave() {

    memset(previousStep, 0, NPOINTS_LOCAL);
    memset(currentStep, 0, NPOINTS_LOCAL);
    memset(nextStep, 0, NPOINTS_LOCAL);

    if (id == MASTER) {
        memset(u_global, 0, NPOINTS_GLOBAL);
    }

    // initialize the first time step
    for (int k = 0; k < NPOINTS_LOCAL; k++ ) {

        double x = (k + left) * DELTA_X;
        previousStep[k] = waveInitFunc(x);
        currentStep[k] = waveInitFunc(x);
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

        double etime = simulateNumberOfTimeSteps();

        if (id == MASTER) {
            runtime[i] = etime;
        }
        resetWave();
    }

    if (id == MASTER) {

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
 
        fprintf(fp, "Running for %5d timesteps with %10d points took %8.5f seconds with stddev = %8.5f after %3d reruns.\n", TPOINTS, NPOINTS_GLOBAL, mean, stddev, RERUNS);
        fclose(fp);
    }

    finalizeWave();
}

void initWaveConditions() {

    // calculate left and right border for every process
    left = (id * (NPOINTS_GLOBAL - 1)) / numberOfProcesses;
    right = ((id + 1) * (NPOINTS_GLOBAL - 1)) / numberOfProcesses;

    if (id != MASTER) {
        left = left - 1;
    }
    NPOINTS_LOCAL = right + 1 - left;

    // allocate space for local arrays
    const size_t bufSize = NPOINTS_LOCAL * sizeof(double);
    previousStep = (double*) malloc(bufSize);
    currentStep = (double*) malloc(bufSize);
    nextStep = (double*) malloc(bufSize);

    // master needs another global array to collect everything in the end
    if (id == MASTER) {
        u_global = (double*) malloc(NPOINTS_GLOBAL * sizeof(double));
    }

    resetWave();
}

void collectWave() {

    // if Master, collect all others
    if (id == MASTER) {

        // write own results to global array first
        for (int i = 0; i <= right; i++ ) {
            u_global[i] = currentStep[i];
        }

        // recieve results from every other process and write it
        for (int i = 1; i < numberOfProcesses; i++ ) {

            // recieve info about the coming data
            CHECK(MPI_Recv(buffer, 2, MPI_INT, i, INFO, MPI_COMM_WORLD, &status));
            
            left = buffer[0]; // start index in global array
            cnt = buffer[1]; // how many points to expect

            // recieve <count> values and write them into the global array, starting at index <left>
            CHECK(MPI_Recv(&u_global[left], cnt, MPI_DOUBLE, i, ACTUAL, MPI_COMM_WORLD, &status));
        }

    } else { // if not master, send to master

        buffer[0] = left; // start index in global array
        buffer[1] = NPOINTS_LOCAL; // how many points to expect
        
        // first send info about the data...
        CHECK(MPI_Send(buffer, 2, MPI_INT, 0, INFO, MPI_COMM_WORLD));

        // ...then send the actual values
        CHECK(MPI_Send(currentStep, NPOINTS_LOCAL, MPI_DOUBLE, 0, ACTUAL, MPI_COMM_WORLD));
    }
}

void finalizeWave() {

    if (id == MASTER) {
        free(u_global);
    }
    free(currentStep);
    free(previousStep);
    free(nextStep);

    // exit mpi
    MPI_Finalize();
}

int getNPOINTS() {
    return NPOINTS_GLOBAL;
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
