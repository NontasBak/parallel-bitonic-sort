#include "../include/bitonic.h"

#include "stdio.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


int main() {

	int p = 3; 
	int q = 3; 
	int num_p = pow(2, p); // 2^p processes
	int num_q = pow(2, q);// 2^q elements per process

	Process* processes;
	makeProcesses(&processes, num_p, num_q);

    printf("Processes initially:");
    printProcesses(processes, num_p, num_q);

	bitonicSort(processes, num_p, num_q);

    printf("Fully sorted array:");
    printProcesses(processes, num_p, num_q);

	freeProcesses(&processes, num_p);

	return 0;

}




