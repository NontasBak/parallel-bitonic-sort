#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/bitonic.h"
#include "time.h"

void makeProcesses(Process** processes, int num_p, int num_q) {
    *processes = (Process*)malloc(sizeof(Process) * num_p);

    for (int i = 0; i < num_p; i++) {
        (*processes)[i].array = (int*)malloc(sizeof(int) * num_q);
        (*processes)[i].id = i;
    }

    srand(time(NULL));

    for (int i = 0; i < num_p; i++) {
        for (int j = 0; j < num_q; j++) {
            (*processes)[i].array[j] = (int)rand() % 100;
        }
    }
}

void freeProcesses(Process** processes, int num_p) {
    for (int i = 0; i < num_p; i++) {
        free((*processes)[i].array);
    }
    free(*processes);
}

int compare_asc(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
int compare_des(const void* a, const void* b) { return (*(int*)b - *(int*)a); }

void printProcesses(Process* processes, int num_p, int num_q) {
    printf("\n");
    for (int i = 0; i < num_p; i++) {
        printf("%d:  ", processes[i].id);
        for (int j = 0; j < num_q; j++) {
            printf("%2d  ", processes[i].array[j]);
        }

        printf("\n");
    }
    printf("\n");
}