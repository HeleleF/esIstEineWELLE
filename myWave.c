/**
 * @file myWave.c
 *
 * This is the main driver of the program, i.e.,
 * the program, which is then used by the user.
 */
#include <stdio.h>
#include <stdlib.h>

#include "core.h"

int main(int argc, char **argv)
{

	getUserInputOrConfig(argc, **argv);

	init();

	simulateNumberOfTimeSteps();

	close();

	return EXIT_SUCCESS;
}
