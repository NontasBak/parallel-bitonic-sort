#include "../include/bitonic.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "time.h"


void makeBitonic(Process** process, int num_p, int num_q) {

    *process = (Process*)malloc(sizeof(Process) * num_p);

    for (int i = 0; i < num_p; i++) {
        (*process)[i].array = (int*)malloc(sizeof(int) * num_q);
        (*process)[i].id = i;
    }

    int elbow = num_q / 2;
    srand(time(NULL));

    for (int i = 0; i < num_p; i++) {
        for (int j = 0; j < num_q; j++) {
            (*process)[i].array[j] = (int)rand() % 100;
        }
        qsort((*process)[i].array, elbow, sizeof(int), compare_asc);
        qsort((*process)[i].array + elbow, num_q - elbow, sizeof(int), compare_des);
    }
}

void freeBitonic(Process** process, int num_p) {

	for (int i = 0; i < num_p; i++) {
		free((*process)[i].array);
	}
	free(*process);
}

int compare_asc(const void* a, const void* b) {
	return (*(int*)a - *(int*)b);
}
int compare_des(const void* a, const void* b) {
	return (*(int*)b - * (int*)a);
}

void printProcess(Process* process, int num_p,int num_q) {

    printf("\n");
	for (int i = 0; i < num_p; i++) {
		printf("%d:  ", process[i].id);
		for (int j = 0; j < num_q; j++) {
			printf("%2d  ", process[i].array[j]);
		}
        if (process[i].id % 2 == 0) {
            if (isSortedAsc(process[i].array, num_q))
                printf("SORTED");
            else
                printf("UNSORTED");
        }
        else {
            if (isSortedDes(process[i].array, num_q))
                printf("SORTED");
            else
                printf("UNSORTED");
        }
            
        printf("\n");
	}
    printf("\n");
}

bool isSortedAsc(const int* arr, int n) {
    for (int i = 1; i < n; i++) {
        if (arr[i] < arr[i - 1]) {
            return false; // Not sorted
        }
    }
    return true; // Sorted
}

bool isSortedDes(const int* arr, int n) {
    for (int i = 1; i < n; i++) {
        if (arr[i] > arr[i - 1]) {
            return false; // Not sorted
        }
    }
    return true; // Sorted
}