/**
 * @file coreMPI.c
 * @author Chris Rebbelin s0548921
 * @date 2018-07-29
 * @brief Contains the main mpi calculation logic for the wave
 * 
 * @details This file implements the main mpi calculation logic for the wave.
 */

#include "coreMPI.h"

// time step arrays
double *previousStep, *currentStep, *nextStep, *globalStep;

// setting values
int intervalEnd, nPointsGlobal, nPointsLocal, tPoints, periods, amplitude, useGui, printvalues, doBenchmark;

const double DELTA_T = 1.0;
double deltaX, lambda, c, cSquared, waveSpeed;

// MPI related values
int id, numberOfProcesses;

// border values
int left, right;

const int FIRST = 0;
int LAST;

MPI_Status status;

// MPI message tags for sending/recieving
const int L_TO_R = 10;
const int R_TO_L = 20;
const int INFO = 30;
const int ACTUAL = 40;

// buffer for sending data info
int buffer[2];

void CHECK(int ierr)
{
    if (ierr != MPI_SUCCESS)
    {
        fprintf(stderr, "MPI_error\n");

        int resultlen;
        char err_buffer[MPI_MAX_ERROR_STRING];

        MPI_Error_string(ierr, err_buffer, &resultlen);
        fprintf(stderr, err_buffer);

        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
}

void outputHelpMessage()
{
    printf("\nHOW TO USE:\n");
    printf("Edit parameters in %s\n", DEFAULT_SETTINGS_FILE_PATH);
    printf("or use your own settings file with 'mpiexec myWaveMPI -c <pathToYourFile>'\n");
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
    printf("From cmd line: 'mpiexec myWaveMPI [-s SPEED] [-t TIMESTEPS] [-i INTERVALEND] [-n POINTS] [-p PERIODS] [-a AMPLITUDE] [-l LAMBDA] [-u SHOWGUI] [-v PRINTVALUES]'\n\n");
    printf("To perform benchmarks, use 'mpiexec myWaveMPI -b TIMESTEPS POINTS' or 'mpiexec myWaveMPI --benchmark TIMESTEPS POINTS'\n");
    printf("To show this message, use 'mpiexec myWaveMPI -h' or 'mpiexec myWaveMPI --help'\n");
}

void getFromSettingsFile(char *configPath)
{
    const unsigned short MAXLINE = 200;
    const unsigned short MAXSETTING = 50;

    FILE *filePointer;
    char buffer[MAXLINE], configKey[MAXSETTING], configValue[MAXSETTING];
    int i, count;

    // try to open the file
    filePointer = fopen(configPath, "r");

    if (NULL == filePointer)
    {
        if (id == FIRST)
        {
            printf("[ERROR] Could not get file '%s'!\n", configPath);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    // read the contents
    while (NULL != fgets(buffer, MAXLINE, filePointer))
    {
        count = 0;
        i = 0;

        // ignore comments or empty lines
        if ('#' == buffer[0] || '\n' == buffer[0])
        {
            continue;
        }

        // get the key
        while (' ' != buffer[i])
        {
            configKey[i] = buffer[i];
            i++;
        }
        configKey[i++] = '\0';
        count = i;

        // get corresponding value
        while ('\n' != buffer[i])
        {
            configValue[i - count] = buffer[i];
            i++;
        }
        configValue[i - count] = '\0';

        // set the corresponding setting
        if (0 == strcmp(configKey, "SPEED"))
        {
            waveSpeed = atof(configValue);
        }
        else if (0 == strcmp(configKey, "NUMBER_OF_TIME_STEPS"))
        {
            tPoints = atoi(configValue);
        }
        else if (0 == strcmp(configKey, "LINE_INTERVAL_END"))
        {
            intervalEnd = atoi(configValue);
        }
        else if (0 == strcmp(configKey, "NUMBER_OF_POINTS"))
        {
            nPointsGlobal = atoi(configValue);
        }
        else if (0 == strcmp(configKey, "SHOW_GUI"))
        {
            useGui = atoi(configValue);
        }
        else if (0 == strcmp(configKey, "NUMBER_OF_PERIODS"))
        {
            periods = atoi(configValue);
        }
        else if (0 == strcmp(configKey, "AMPLITUDE"))
        {
            amplitude = atoi(configValue);
        }
        else if (0 == strcmp(configKey, "LAMBDA"))
        {
            lambda = atof(configValue);
        }
        else if (0 == strcmp(configKey, "PRINT_VALUES"))
        {
            printvalues = atoi(configValue);
        }
        else
        {
            if (id == FIRST)
            {
                printf("[INFO] Unrecognized settings key: '%s'\n", configKey);
            }
        }
    }

    fclose(filePointer);
}

void getFromCmdLine(int nargc, char **argv)
{

    // check cmdline arguments
    if (0 == strcmp(argv[1], "-c") || 0 == strcmp(argv[1], "--use-config-file"))
    {

        // use custom settings file
        getFromSettingsFile(argv[2]);
    }
    else if (0 == strcmp(argv[1], "-h") || 0 == strcmp(argv[1], "--help"))
    {

        if (id == FIRST)
        {
            outputHelpMessage();
        }
        MPI_Finalize();
        exit(EXIT_SUCCESS);
    }
    else if (0 == strcmp(argv[1], "-v") || 0 == strcmp(argv[1], "--version"))
    {

        if (id == FIRST)
        {
            printf("Wave equation MPI - Psys18\nChris Rebbelin 2018\nVersion 0.1\n");
        }
        MPI_Finalize();
        exit(EXIT_SUCCESS);
    }
    else if (0 == strcmp(argv[1], "-b") || 0 == strcmp(argv[1], "--benchmark"))
    {

        if (nargc == 4)
        {

            doBenchmark = 1;

            // only number of timesteps and points can be set in benchmarks
            // all other values are default
            tPoints = atoi(argv[2]);
            nPointsGlobal = atoi(argv[3]);

            // visualization is disabled
            useGui = 0;
        }
        else
        {

            // print help message and exit
            if (id == FIRST)
            {
                outputHelpMessage();
            }
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }
    }
    else
    {

        for (int i = 1; i < nargc; i++)
        {

            if (0 == strcmp(argv[i], "-s") || 0 == strcmp(argv[i], "--speed"))
            {

                waveSpeed = atof(argv[++i]);
            }
            else if (0 == strcmp(argv[i], "-t") || 0 == strcmp(argv[i], "--timesteps"))
            {

                tPoints = atoi(argv[++i]);
            }
            else if (0 == strcmp(argv[i], "-i") || 0 == strcmp(argv[i], "--intervalend"))
            {

                intervalEnd = atoi(argv[++i]);
            }
            else if (0 == strcmp(argv[i], "-n") || 0 == strcmp(argv[i], "--npoints"))
            {

                nPointsGlobal = atoi(argv[++i]);
            }
            else if (0 == strcmp(argv[i], "-p") || 0 == strcmp(argv[i], "--periods"))
            {

                periods = atof(argv[++i]);
            }
            else if (0 == strcmp(argv[i], "-a") || 0 == strcmp(argv[i], "--amplitude"))
            {

                amplitude = atof(argv[++i]);
            }
            else if (0 == strcmp(argv[i], "-l") || 0 == strcmp(argv[i], "--lambda"))
            {

                lambda = atof(argv[++i]);
            }
            else if (0 == strcmp(argv[i], "-u") || 0 == strcmp(argv[i], "--usegui"))
            {

                useGui = atoi(argv[++i]);
            }
            else if (0 == strcmp(argv[i], "-v") || 0 == strcmp(argv[i], "--printvalues"))
            {

                printvalues = atoi(argv[++i]);
            }
            else
            {

                if (id == FIRST)
                {
                    printf("[INFO] Unrecognized argument: %s\n", argv[i]);
                }
            }
        }
    }
}

void checkParams()
{

    if (waveSpeed <= 0 || waveSpeed >= 1)
    {
        if (id == FIRST)
        {
            printf("[ERROR] Wave equation not stable with c=%.3f!\n", waveSpeed);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (tPoints < 0)
    {
        if (id == FIRST)
        {
            printf("[ERROR] Number of time steps must not be negative!\n");
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (intervalEnd < 1)
    {
        if (id == FIRST)
        {
            printf("[ERROR] Right interval border must not be smaller than 1\n");
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (nPointsGlobal <= 0)
    {
        if (id == FIRST)
        {
            printf("[ERROR] Number of discrete points must not be negative or zero!\n");
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (nPointsGlobal > MAX_POINTS)
    {
        if (id == FIRST)
        {
            printf("[ERROR] Number of discrete points is bigger than the allowed maximum of %d!\n", MAX_POINTS);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (periods < 1)
    {
        if (id == FIRST)
        {
            printf("[ERROR] Period length must not be smaller than 1!\n");
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (amplitude < 1)
    {
        if (id == FIRST)
        {
            printf("[ERROR] Amplitude must not be smaller than 1!\n");
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (lambda < 0)
    {
        if (id == FIRST)
        {
            printf("[ERROR] Dampening factor must not be negative!\n");
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (lambda > MAX_LAMBDA)
    {
        if (id == FIRST)
        {
            printf("[ERROR] Dampening factor must not be greater than %.2f!\n", MAX_LAMBDA);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (!useGui && tPoints == 0)
    {
        if (id == FIRST)
        {
            printf("[ERROR] Simulating an endless loop is only allowed with visualisation enabled!\n");
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    deltaX = (intervalEnd / nPointsGlobal);

    c = (DELTA_T / deltaX) * waveSpeed;
    cSquared = c * c;

    if (id == FIRST)
    {

        if (tPoints == 0)
        {
            printf("Looping forever");
        }
        else
        {
            printf("Simulating %d time steps", tPoints);
        }
        printf(" with parameters:\n");
        printf("Using %d discrete points in line interval [0,%d] with speed %.3f\n", nPointsGlobal, intervalEnd, waveSpeed);
        printf("Simulating a %sdampened", lambda == 0 ? "un" : "");
        printf(" sine wave with amplitude %d and %d periods", amplitude, periods);
        if (lambda != 0)
        {
            printf(" and damping factor %f", lambda);
        }

        printf("\n");
    }
}

void getUserInputOrConfig(int numberofargc, char **argv, int pid, int pnum)
{

    if (pnum < 2) {
        if (pid == FIRST)
        {
            printf("[ERROR] At least 2 processes needed!\n");
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    // set id and number of processes once
    // so subsequent functions don't need to pass those values again
    id = pid;
    numberOfProcesses = pnum;

    LAST = pnum - 1;

    // set the default values
    // will be used if not overwritten
    waveSpeed = 0.9;
    tPoints = 1000;
    intervalEnd = 1000;
    nPointsGlobal = 1000;
    periods = 5;
    amplitude = intervalEnd / 4;
    lambda = 0;
    useGui = 1;
    printvalues = 0;

    if (numberofargc > 1)
    {
        getFromCmdLine(numberofargc, argv);
    }
    else
    {
        // no cmd args, use default settings file
        getFromSettingsFile(DEFAULT_SETTINGS_FILE_PATH);
    }

    // check values for validity
    checkParams();
}

double waveInitFunc(double x)
{
    // divide by (intervalEnd - 1) to satisfy the right border condition
    // necessary because 1000 points means indices 0 to 999
    return (amplitude * sin(2 * x * M_PI * periods / (intervalEnd - 1)));
}

void initWaveConditions()
{
    // calculate left and right border for every process
    left = (id * (nPointsGlobal - 1)) / numberOfProcesses;
    right = ((id + 1) * (nPointsGlobal - 1)) / numberOfProcesses;

    if (id != FIRST)
    {
        left = left - 1;
    }
    nPointsLocal = right + 1 - left;

    // allocate space for local arrays
    const size_t bufSize = nPointsLocal * sizeof(double);
    previousStep = (double *) malloc(bufSize);
    currentStep = (double *) malloc(bufSize);
    nextStep = (double *) malloc(bufSize);

    // master needs another global array to collect everything in the end
    if (id == FIRST)
    {
        globalStep = (double *) malloc(nPointsGlobal * sizeof(double));
    }

    resetWave();
}

void simulateOneTimeStep()
{

    int i;

    // calculate next time step with wave equation
    for (i = 1; i < right - left; i++)
    {
        nextStep[i] = 2.0 * currentStep[i] - previousStep[i] + cSquared * (currentStep[i - 1] - (2.0 * currentStep[i]) + currentStep[i + 1]);
    }

    if (id != FIRST)
    {
        // exchange border values with the left neighbor
        CHECK(MPI_Send(&nextStep[1], 1, MPI_DOUBLE, id - 1, R_TO_L, MPI_COMM_WORLD));
        CHECK(MPI_Recv(&nextStep[0], 1, MPI_DOUBLE, id - 1, L_TO_R, MPI_COMM_WORLD, &status));
    }
    else
    {
        // FIRST is the "leftmost" process and has no left neighbor but the boundary condition
        nextStep[0] = 0.0;
    }

    if (id != LAST)
    {
        // exchange border values with the right neighbor
        CHECK(MPI_Send(&nextStep[right - left - 1], 1, MPI_DOUBLE, id + 1, L_TO_R, MPI_COMM_WORLD));
        CHECK(MPI_Recv(&nextStep[right - left], 1, MPI_DOUBLE, id + 1, R_TO_L, MPI_COMM_WORLD, &status));
    }
    else
    {
        // LAST is the "rightmost" and has no right neighbor but the boundary condition
        nextStep[right - left] = 0.0;
    }

    // copy values one step "into the past"
    double *tempStep = previousStep;
    previousStep = currentStep;
    currentStep = nextStep;
    nextStep = tempStep;
}

double simulateNumberOfTimeSteps()
{

    double elapsed, wtime = MPI_Wtime();

    // time steps
    for (int i = 1; i < tPoints; ++i)
    {
        simulateOneTimeStep();
    }
    collectWave();

    elapsed = MPI_Wtime() - wtime;

    if (id == FIRST && printvalues)
    {
        outputNew();
    }

    return elapsed;
}

void collectWave()
{

    // if Master, collect all others
    if (id == FIRST)
    {

        // write own results to global array first
        for (int i = 0; i <= right; i++)
        {
            globalStep[i] = currentStep[i];
        }

        int startIdx, cnt = 0;

        // recieve results from every other process and write it
        for (int i = 1; i < numberOfProcesses; i++)
        {

            // recieve info about the coming data
            CHECK(MPI_Recv(buffer, 2, MPI_INT, i, INFO, MPI_COMM_WORLD, &status));

            startIdx = buffer[0]; // start index in global array
            cnt = buffer[1];      // how many points to expect

            // recieve <count> values and write them into the global array, starting at index <left>
            CHECK(MPI_Recv(&globalStep[startIdx], cnt, MPI_DOUBLE, i, ACTUAL, MPI_COMM_WORLD, &status));
        }
        //outputNew();
    }
    else
    { // if not master, send to master

        buffer[0] = left;         // start index in global array
        buffer[1] = nPointsLocal; // how many points to expect

        // first send info about the data...
        CHECK(MPI_Send(buffer, 2, MPI_INT, 0, INFO, MPI_COMM_WORLD));

        // ...then send the actual values
        CHECK(MPI_Send(currentStep, nPointsLocal, MPI_DOUBLE, 0, ACTUAL, MPI_COMM_WORLD));
    }
}

void finalizeWave()
{

    if (id == FIRST)
    {
        free(globalStep);
    }
    free(currentStep);
    free(previousStep);
    free(nextStep);

    // exit mpi
    MPI_Finalize();
}

void resetWave()
{

    memset(previousStep, 0, nPointsLocal);
    memset(currentStep, 0, nPointsLocal);
    memset(nextStep, 0, nPointsLocal);

    if (id == FIRST)
    {
        memset(globalStep, 0, nPointsGlobal);
    }

    // initialize the first time step
    for (int k = 0; k < nPointsLocal; k++)
    {
        double x = (k + left) * deltaX;
        previousStep[k] = waveInitFunc(x);
        currentStep[k] = waveInitFunc(x);
    }
}

void outputNew()
{

    printf("####Current Values:####\n");
    for (int l = 0; l < nPointsGlobal; ++l)
    {
        printf("%4d => %6.6f\n", l, globalStep[l]);
    }
}

void performBenchmark()
{

    const unsigned short RERUNS = 10;

    double runtime[RERUNS];

    memset(runtime, 0, RERUNS);

    double mean = 0.0;
    double stddev = 0.0;

    initWaveConditions();

    // run repeatedly
    for (int i = 0; i < RERUNS; i++)
    {
        double etime = simulateNumberOfTimeSteps();

        if (id == FIRST)
        {
            runtime[i] = etime;
        }
        resetWave();
    }

    if (id == FIRST)
    {

        FILE *fp;
        fp = fopen(BENCHMARK_FILE, "a");

        if (NULL == fp)
        {
            printf("[ERROR] Could not get file '%s'!\n", BENCHMARK_FILE);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            exit(EXIT_FAILURE);
        }

        // calculate run time statistics
        for (int i = 0; i < RERUNS; i++)
        {
            fprintf(fp, "Run %2d: %10.8f seconds\n", i, runtime[i]);
            mean += runtime[i];
        }
        mean = mean / RERUNS;

        for (int i = 0; i < RERUNS; i++)
        {
            stddev += pow(runtime[i] - mean, 2);
        }
        stddev = sqrt(stddev / RERUNS);

        printf("Finished! Mean: %10.8f (Stddev:%10.8f)\n", mean, stddev);

        fprintf(fp, "Running for %5d timesteps with %10d points took %10.8f seconds with stddev = %10.8f after %2d reruns.\n", tPoints, nPointsGlobal, mean, stddev, RERUNS);
        fclose(fp);
    }

    finalizeWave();
}

double *getStep()
{
    return globalStep;
}

int getNpoints()
{
    return nPointsGlobal;
}

int getTpoints()
{
    return tPoints;
}

double getLambda()
{
    return lambda;
}

int showGui()
{
    return useGui;
}

int doBench()
{
    return doBenchmark;
}
