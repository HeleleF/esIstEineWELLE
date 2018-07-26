/**
 * Implements the main program and visualisation to the user
 *
 * @file myWaveMPI.c
 * @author Chris Rebbelin s0548921
 * @date 2018-07-01
 * @brief This file contains the main MPI program
 */

#include "myWaveMPI.h"

int main(int argc, char **argv) {

    const int MASTER = 0; 

    int id, numberOfProcesses;

    // init mpi
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

    // init mpi error handler
    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    getUserInputOrConfig(argc, argv, id, numberOfProcesses);

    if (doBench()) {
        performBenchmark();
        return EXIT_SUCCESS;
    }

    if (useGUI()) {

        initWaveConditions();

        printf("\n Controls:\n\tA\t\ttoggle axis\n");
        printf("\tP\t\tpause / continue the visualisation\n");
        printf("\tR\t\treset to initial sine wave\n");
        printf("\tQ / ESC\t\tquit the program\n");

        //doGraphics();

    } else {

        double waveTime;

        initWaveConditions();
        waveTime = simulateNumberOfTimeSteps();

        if (id == MASTER) {
            printf ("Elapsed wallclock time was %g seconds\n", waveTime);
        }
        
        finalizeWave();

    }
    return EXIT_SUCCESS;
}
