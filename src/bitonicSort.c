#include "../include/bitonic.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


void bitonicSort(Process* processes, int num_p, int num_q) {
    // Initial sorting of the processes
	sortProcesses(processes, num_p, num_q, 1);

	for (int step = 2; step <= num_p; step *= 2) { // step defines the external reccursion (block size of merge_up or down)
		for (int i = 0; i < num_p; i += step * 2) { // j defines the blocks for exchange (start_id)
			merge_up(processes, i, step, num_q);
			if (step + i < num_p) {
				merge_down(processes, step + i, step, num_q);
			}
		}
        printf("\nCheck if the processes are bitonic before sorting:");
        printProcesses(processes, num_p, num_q);
        
        sortProcesses(processes, num_p, num_q, step);
	}	
}


//size is the size of block that calls merge_up 
void merge_up(Process* processes, int start_id, int size, int num_q) {
	
	int cut = size/2;

	//comparison
	for (int i = 0; i < cut; i++) {
		compare_up_elementwise(processes[start_id + i], processes[start_id + cut + i], num_q);
		printf("Merge up %d, %d", start_id + i, start_id + cut + i);
		printf("\n");
	}
	if (cut == 1) {
		return;
	}
	
	merge_up(processes, start_id, cut, num_q);
	merge_up(processes, start_id + cut, cut, num_q);
}

void merge_down(Process* processes, int start_id, int size, int num_q) {

	int cut = size / 2;

	//comparison
	for (int i = 0; i < cut; i++) {
		compare_down_elementwise(processes[start_id + i], processes[start_id + cut + i], num_q);
		printf("Merge down %d, %d", start_id + i, start_id + cut + i);
		printf("\n");
	}
	if (cut == 1) {
		return;
	}

	merge_down(processes, start_id, cut, num_q);
	merge_down(processes, start_id + cut, cut, num_q);
}

void compare_up_elementwise(Process p1, Process p2, int num_q) {
    for(int i = 0; i < num_q; i++) {
        if(p1.array[i] > p2.array[i]) {
            int temp = p1.array[i];
            p1.array[i] = p2.array[i];
            p2.array[i] = temp;
        }
    }
}


void compare_down_elementwise(Process p1, Process p2, int num_q) {
    for (int i = 0; i < num_q; i++) {
        if (p1.array[i] < p2.array[i]) {
            int temp = p1.array[i];
            p1.array[i] = p2.array[i];
            p2.array[i] = temp;
        }
    }
}

void sortProcesses(Process* processes, int num_p, int num_q, int size) {
    // This function should implement the "Elbow merge" sorting method.
    // The process should be a bitonic.
    // The sorting algorithm is not implemented at the moment.

    for (int i = 0; i < num_p; i++) {
        // printf("\n %d %d %d", i, i % (size * 2), size);
        if (i % (size * 2) < size) {
            qsort(processes[i].array, num_q, sizeof(int), compare_asc);
        }
        else {
            qsort(processes[i].array, num_q, sizeof(int), compare_des);
        }
    }
}
