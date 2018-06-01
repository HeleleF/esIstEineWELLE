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

	init();

	simulateNumberOfTimeSteps(10);

	close();

	return EXIT_SUCCESS;
}
