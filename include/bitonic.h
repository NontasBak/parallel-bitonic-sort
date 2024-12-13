#ifndef BITONIC_H
#define BITONIC_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int *array;
    int id;
} Process;

void bitonicSort(Process *processes, int num_p, int num_q);
void merge_up(Process *processes, int start_id, int size, int num_q);
void merge_down(Process *processes, int start_id, int size, int num_q);
void compare_up_elementwise(Process p1, Process p2, int num_q);
void compare_down_elementwise(Process p1, Process p2, int num_q);

void sortProcesses(Process *processes, int num_p, int num_q, int size);
void makeProcesses(Process **processes, int num_p, int num_q);
void freeProcesses(Process **processes, int num_p);

int compare_asc(const void *a, const void *b);
int compare_des(const void *a, const void *b);
void printProcesses(Process *processes, int num_p, int num_q);

#endif
