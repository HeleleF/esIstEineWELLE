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

    int id;
    int numberOfProcesses;

    double wtime;

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &id );
    MPI_Comm_size ( MPI_COMM_WORLD, &numberOfProcesses );

    const int MASTER = 0;

    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    if (id == MASTER) {
      getUserInputOrConfig(argc, argv);
    }

    if (useGUI()) {

        initWaveConditions(id, numberOfProcesses);

        printf("\n Controls:\n\tA\t\ttoggle axis\n");
        printf("\tP\t\tpause / continue the visualisation\n");
        printf("\tR\t\treset to initial sine wave\n");
        printf("\tQ / ESC\t\tquit the program\n");

        doGraphics();

    } else {

        wtime = MPI_Wtime();
        initWaveConditions(id, numberOfProcesses);
        simulateNumberOfTimeSteps();

        if (id == MASTER) {
            printf ("Elapsed wallclock time was %g seconds\n", MPI_Wtime() - wtime);
        }

        finalizeWave(id, numberOfProcesses);

    }
    return EXIT_SUCCESS;
}
