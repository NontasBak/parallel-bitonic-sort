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

	Process* process;
	makeBitonic(&process, num_p, num_q);

	bitonicSort(process, num_p);

	freeBitonic(&process, num_p);

	return 0;

}




