#ifndef BITONIC_H
#define BITONIC_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
	int* array;
	int id;
}Process;


void bitonicSort(Process* process, int num_p);
void merge_up(Process* process, int start_id, int size);
void merge_down(Process* process, int start_id, int size);
void compare_up(Process p1, Process p2);
void compare_down(Process p1, Process p2);

void makeBitonic(Process** process, int num_p, int num_q);
void freeBitonic(Process** process, int num_p);
int compare_asc(const void* a, const void* b);
int compare_des(const void* a, const void* b);
void printProcess(Process* process, int num_p, int num_q);
bool isSortedAsc(const int* arr, int n);
bool isSortedDes(const int* arr, int n);

#endif
