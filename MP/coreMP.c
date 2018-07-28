/**
 * @file coreMP.c
 * @author Chris Rebbelin s0548921
 * @date 2018-07-01
 * @brief Contains the main calculation logic for the wave
 * 
 * @details This file implements the main calculation logic for the wave.
 */

#include <omp.h>
#include "coreMP.h"

// time step arrays
double *previousStep, *currentStep, *nextStep;

// setting values
int intervalEnd, nPoints, tPoints, periods, amplitude, useGui, printvalues, doBenchmark;

const double DELTA_T = 1.0;
double deltaX, lambda, c, cSquared, waveSpeed;

void outputHelpMessage()
{
    printf("\nHOW TO USE:\n");
    printf("Edit parameters in %s\n", DEFAULT_SETTINGS_FILE_PATH);
    printf("or use your own settings file with './myWaveMP -c <pathToYourFile>'\n");
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
    printf("From cmd line: './myWaveMP [-s SPEED] [-t TIMESTEPS] [-i INTERVALEND] [-n POINTS] [-p PERIODS] [-a AMPLITUDE] [-l LAMBDA] [-u SHOWGUI] [-v PRINTVALUES]'\n\n");
    printf("To perform benchmarks, use './myWaveMP -b TIMESTEPS POINTS' or './myWaveMP --benchmark TIMESTEPS POINTS'\n");
    printf("To show this message, use './myWaveMP -h' or './myWaveMP --help'\n");
}

void getFromSettingsFile(char *configPath)
{
    const int MAXLINE = 200;

    FILE *filePointer;
    char buffer[MAXLINE], configKey[50], configValue[50];
    int i, count;

    // try to open the file
    filePointer = fopen(configPath, "r");
    if (NULL == filePointer)
    {
        printf("[ERROR] Could not get file '%s'!\n", configPath);
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
            nPoints = atoi(configValue);
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
            printf("[INFO] Unrecognized settings key: '%s'\n", configKey);
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

        // print help message and exit
        outputHelpMessage();
        exit(EXIT_SUCCESS);
    }
    else if (0 == strcmp(argv[1], "-v") || 0 == strcmp(argv[1], "--version"))
    {

        // print version and exit
        printf("Wave equation MP - Psys18\nChris Rebbelin 2018\nVersion 0.1\n");
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
            nPoints = atoi(argv[3]);

            // visualization is disabled
            useGui = 0;
        }
        else
        {

            // print help message and exit
            outputHelpMessage();
            exit(EXIT_FAILURE);
        }
    }
    else
    {

        // iterate over all argv
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

                nPoints = atoi(argv[++i]);
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

                printf("Unrecognized argument: %s\n", argv[i]);
            }
        }
    }
}

void checkParams()
{

    if (waveSpeed <= 0 || waveSpeed >= 1)
    {
        printf("[ERROR] Wave equation not stable with c=%.3f!\n", waveSpeed);
        exit(EXIT_FAILURE);
    }

    if (tPoints < 0)
    {
        printf("[ERROR] Number of time steps must not be negative!\n");
        exit(EXIT_FAILURE);
    }

    if (intervalEnd < 1)
    {
        printf("[ERROR] Right interval border must not be smaller than 1\n");
        exit(EXIT_FAILURE);
    }

    if (nPoints <= 0)
    {
        printf("[ERROR] Number of discrete points must not be negative or zero!\n");
        exit(EXIT_FAILURE);
    }

    if (nPoints > MAX_POINTS)
    {
        printf("[ERROR] Number of discrete points is bigger than the allowed maximum of %d!\n", MAX_POINTS);
        exit(EXIT_FAILURE);
    }

    if (periods < 1)
    {
        printf("[ERROR] Period length must not be smaller than 1!\n");
        exit(EXIT_FAILURE);
    }

    if (amplitude < 1)
    {
        printf("[ERROR] Amplitude must not be smaller than 1!\n");
        exit(EXIT_FAILURE);
    }

    if (lambda < 0)
    {
        printf("[ERROR] Damping factor must not be negative!\n");
        exit(EXIT_FAILURE);
    }

    if (lambda > MAX_LAMBDA)
    {
        printf("[ERROR] Damping factor must not be greater than %.2f!\n", MAX_LAMBDA);
        exit(EXIT_FAILURE);
    }

    if (!useGui && tPoints == 0)
    {
        printf("[ERROR] Simulating an endless loop is only allowed with visualisation enabled!\n");
        exit(EXIT_FAILURE);
    }

    // calculate delta x
    deltaX = (intervalEnd / nPoints);

    // calculate c^2
    c = (DELTA_T / deltaX) * waveSpeed;
    cSquared = c * c;

    if (tPoints == 0)
    {
        printf("Looping forever");
    }
    else
    {
        printf("Simulating %d time steps", tPoints);
    }
    printf(" with parameters:\n");
    printf("Using %d discrete points in line interval [0,%d] with speed %.3f\n", nPoints, intervalEnd, waveSpeed);
    printf("Simulating a %sdampened", lambda == 0 ? "un" : "");
    printf(" sine wave with amplitude %d and %d periods", amplitude, periods);
    if (lambda != 0)
        printf(" and damping factor %f", lambda);
    printf("\n");
}

void getUserInputOrConfig(int numberofargc, char **argv)
{

    // default values
    waveSpeed = 0.9;
    tPoints = 1000;
    intervalEnd = 1000;
    nPoints = 1000;
    periods = 5;
    amplitude = intervalEnd / 4;
    lambda = 0;
    useGui = 1;
    printvalues = 0;
    doBenchmark = 0;

    if (numberofargc > 1)
    {

        getFromCmdLine(numberofargc, argv);
    }
    else
    {

        // use default settings file
        getFromSettingsFile(DEFAULT_SETTINGS_FILE_PATH);
    }

    checkParams();
}

double waveInitFunc(double x)
{
    return (amplitude * sin(2 * x * M_PI * periods / (intervalEnd - 1)));
}

void initWaveConditions()
{

    // initialize arrays
    const size_t bufSize = (nPoints) * sizeof(double);
    previousStep = malloc(bufSize);
    currentStep = malloc(bufSize);
    nextStep = malloc(bufSize);

    resetWave();
}

void simulateOneTimeStep(int holdflag)
{

    int i;

    #pragma omp parallel for shared(nextStep, currentStep, previousStep, cSquared, nPoints, holdflag) private(i)
    for (i = 1; i < nPoints - 1; i++)
    {

        if (holdflag == i)
        {
            // Point at holdflag is fixed, so don't calculate a new position for it, just use the old one
            nextStep[holdflag] = currentStep[holdflag];
        }
        else
        {
            nextStep[i] = 2.0 * currentStep[i] - previousStep[i] + cSquared * (currentStep[i - 1] - (2.0 * currentStep[i]) + currentStep[i + 1]);
        }
    }

    // update boundary conditions
    nextStep[0] = 0.0;
    nextStep[nPoints - 1] = 0.0;

    // copy values one step "into the past"
    double *tempStep = previousStep;
    previousStep = currentStep;
    currentStep = nextStep;
    nextStep = tempStep;
}

double simulateNumberOfTimeSteps()
{

    double start = omp_get_wtime();

    for (int i = 1; i < tPoints; ++i)
    {
        simulateOneTimeStep(0);
    }

    double end = omp_get_wtime();

    if (printvalues)
    {
        outputNew();
    }

    return (end - start);
}

void finalizeWave()
{

    free(previousStep);
    free(currentStep);
    free(nextStep);
}

void resetWave()
{

    double x;
    int i;

    memset(previousStep, 0, nPoints);
    memset(currentStep, 0, nPoints);
    memset(nextStep, 0, nPoints);

    for (i = 0; i < nPoints; i++)
    {

        x = i * deltaX;
        previousStep[i] = waveInitFunc(x);
        currentStep[i] = waveInitFunc(x);
    }
}

void outputNew()
{

    printf("####Current Values:####\n");
    for (int l = 0; l < nPoints; ++l)
    {
        printf("%4d => %6.6f\n", l, currentStep[l]);
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
        runtime[i] = simulateNumberOfTimeSteps();
        resetWave();
    }

    finalizeWave();

    FILE *fp;
    fp = fopen(BENCHMARK_FILE, "a");

    if (NULL == fp)
    {
        printf("[ERROR] Could not get file '%s'!\n", BENCHMARK_FILE);
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

    fprintf(fp, "Running for %5d timesteps with %10d points took %10.8f seconds with stddev = %10.8f after %2d reruns.\n", tPoints, nPoints, mean, stddev, RERUNS);
    fclose(fp);
}

double *getStep()
{
    return currentStep;
}

int getNpoints()
{
    return nPoints;
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
