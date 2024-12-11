#include "../include/bitonic.h"

#include <stdio.h>


void bitonicSort(int num_p) {
	
	for (int i = 2; i <= num_p; i *= 2) { // i defines the external reccursion (block size of merge_up or down)
		for (int j = 0; j < num_p; ) { // j defines the blocks for exchange (start_id)
			merge_up(j, i);
			if (j + i < num_p) {
				merge_down(j + i, i);
			}
			j += i * 2;
		}
	}	
}


//size is the size of block that calls merge_up 
void merge_up(int start_id, int size) {
	
	int cut = size/2;

	//comparison
	for (int i = 0; i < cut; i++) {
		compare_up(start_id + i, start_id + cut + i);
		printf("%d, %d", start_id + i, start_id + cut + i);
		printf("\n");

	}
	printf("\n");
	if (cut == 1) {
		return;
	}
	

	merge_up(start_id, cut);
	merge_up(start_id + cut, cut);

}

void merge_down(int start_id, int size) {

	int cut = size / 2;

	//comparison
	for (int i = 0; i < cut; i++) {
		compare_down(start_id + i, start_id + cut + i);
		printf("%d, %d", start_id + i, start_id + cut + i);
		printf("\n");
	}
	printf("\n");
	if (cut == 1) {
		return;
	}


	merge_down(start_id, cut);
	merge_down(start_id + cut, cut);
}

void compare_up(int id1, int id2) {

}


void compare_down(int id1, int id2) {

}
