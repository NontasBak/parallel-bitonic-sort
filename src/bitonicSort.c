#include "../include/bitonic.h"

#include <stdio.h>


void merge(int* array, int size, int id) {

	if (id % 2 == 0)
		merge_up(array, 0, size - 1);
	else
		merge_down(array, 0, size - 1);
}


void merge_up(int* array, int low, int high) {

	int length = high - low + 1;
	int cut = length / 2;
	if (cut == 0) {
		return;
	}
	for (int i = 0; i < cut; i++) {
		if (array[i + low] > array[low + cut + i]) {
			int temp = array[i + low];
			array[i + low] = array[low + cut + i];
			array[low + cut + i] = temp;
		}
	}
	merge_up(array, low, low + cut - 1);
	merge_up(array, low + cut, high);

}

void merge_down(int* array, int low, int high) {

	int length = high - low + 1;
	int cut = length / 2;
	if (cut == 0) {
		return;
	}
	for (int i = 0; i < cut; i++) {
		if (array[i + low] < array[low + cut + i]) {
			int temp = array[i + low];
			array[i + low] = array[low + cut + i];
			array[low + cut + i] = temp;
		}
	}
	merge_down(array, low, low + cut - 1);
	merge_down(array, low + cut, high);
}


