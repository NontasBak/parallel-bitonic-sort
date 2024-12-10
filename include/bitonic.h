#ifndef BITONIC_H
#define BITONIC_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
	int* array;
	int id;
}Process;


void merge(int* array, int size, int id);
void merge_up(int* array, int low, int high);
void merge_down(int* array, int low, int high);

void makeBitonic(Process** process, int num_p, int num_q);
void freeBitonic(Process** process, int num_p);
int compare_asc(const void* a, const void* b);
int compare_des(const void* a, const void* b);
void printProcess(Process* process, int num_p, int num_q);
bool isSortedAsc(const int* arr, int n);
bool isSortedDes(const int* arr, int n);

#endif
