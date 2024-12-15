#ifndef BITONIC_H
#define BITONIC_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int *array;
    int id;
} Process;

void bitonicSort(int rank, int num_p, int num_q, int *array);
void merge_up(int rank, int start_id, int size, int num_q, int *array);
void merge_down(int rank, int start_id, int size, int num_q, int *array);
void compare_up_elementwise(int rank, int rank_p1, int rank_p2, int num_q,
                            int *array);
void compare_down_elementwise(int rank, int rank_p1, int rank_p2, int num_q,
                              int *array);
void keepMinElements(int *array, int *recv_array, int num_q);
void keepMaxElements(int *array, int *recv_array, int num_q);

void sortProcesses(int rank, int num_p, int num_q, int size, int *array);
void makeProcesses(Process **processes, int num_p, int num_q);
void freeProcesses(Process **processes, int num_p);

int compare_asc(const void *a, const void *b);
int compare_des(const void *a, const void *b);
void printProcesses(Process *processes, int num_p, int num_q);

#endif
