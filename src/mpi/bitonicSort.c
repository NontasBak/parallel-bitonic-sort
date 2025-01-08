#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/bitonic.h"

void bitonicSort(int rank, int num_p, int num_q, int **array) {
    // Initial sorting of the processes
    sortProcesses(rank, num_q, *array);

    for (int group_size = 2; group_size <= num_p; group_size *= 2) {
        bool sort_descending = rank & group_size;
        for (int distance = group_size / 2; distance > 0; distance /= 2) {
            int partner_rank = rank ^ distance;

            minmax(rank, partner_rank, num_q, *array, sort_descending);
        }
        elbowMerge(num_p, num_q, array, sort_descending);
    }
}

void minmax(int rank, int partner_rank, int num_q, int *array,
            bool sort_descending) {
    int *recv_array = (int *)malloc(num_q * sizeof(int));

    MPI_Sendrecv(array, num_q, MPI_INT, partner_rank, 0, recv_array, num_q,
                 MPI_INT, partner_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if ((!sort_descending && rank < partner_rank) ||
        (sort_descending && rank > partner_rank)) {
        keepMinElements(array, recv_array, num_q);
    } else {
        keepMaxElements(array, recv_array, num_q);
    }
    free(recv_array);
}

void sortProcesses(int rank, int num_q, int *array) {
    // Even processes are ascending sorted
    // Odd processes are descending sorted
    if (rank % 2 == 0) {
        qsort(array, num_q, sizeof(int), compare_asc);
    } else {
        qsort(array, num_q, sizeof(int), compare_des);
    }
}

void keepMinElements(int *array, int *recv_array, int num_q) {
    for (int i = 0; i < num_q; i++) {
        if (array[i] > recv_array[i]) {
            array[i] = recv_array[i];
        }
    }
}

void keepMaxElements(int *array, int *recv_array, int num_q) {
    for (int i = 0; i < num_q; i++) {
        if (array[i] < recv_array[i]) {
            array[i] = recv_array[i];
        }
    }
}

void elbowMerge(int num_p, int num_q, int **array, bool sort_descending) {
    int elbow = findElbow(num_q, *array);  // index of elbow

    int left = elbow;
    int right = (left == num_q - 1) ? 0 : (left + 1);

    // Sorted array
    int *sorted = (int *)malloc(num_q * sizeof(int));
    if (sorted == NULL) {
        printf("Error by allocating memory!");
        return;
    }

    if (sort_descending) {
        for (int index = num_q - 1; index >= 0; index--) {
            compareElements(&left, &right, index, num_q, *array, sorted);
        }
    } else {
        for (int index = 0; index < num_q; index++) {
            compareElements(&left, &right, index, num_q, *array, sorted);
        }
    }

    free(*array);
    *array = sorted;
}

// finds the min element as elbow
int findElbow(int num_q, int *array) {
    int min = 0;  // first element as elbow

    for (int i = 1; i < num_q; i++) {
        if (array[min] > array[i]) {
            min = i;
        }
    }
    return min;
}

// Places the min element between left and right into the index position
void compareElements(int *left, int *right, int index, int num_q, int *array,
                     int *sorted) {
    int l = (*left);
    int r = (*right);

    if (array[r] <= array[l]) {
        sorted[index] = array[r];
        (*right) = (r == num_q - 1) ? 0 : (r + 1);
    } else {
        sorted[index] = array[l];
        (*left) = (l == 0) ? (num_q - 1) : (l - 1);
    }
}

// helpers for qsort
int compare_asc(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}
int compare_des(const void *a, const void *b) {
    return (*(int *)b - *(int *)a);
}

void print(int rank, int size, int num_q, int *array) {
    MPI_Barrier(MPI_COMM_WORLD);
    for (int i = 0; i < size; i++) {
        if (rank == i) {
            printf("Process %d: ", rank);
            for (int j = 0; j < num_q; j++) {
                printf("%2d ", array[j]);
            }
            MPI_Barrier(MPI_COMM_WORLD);
            printf("\n");
            fflush(stdout);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

void evaluateResult(int rank, int num_p, int num_q, int *array) {
    int last_element = array[num_q - 1];
    int next_first_element;
    bool is_sorted = true;

    // Check if the local array is sorted
    for (int i = 1; i < num_q; i++) {
        if (array[i - 1] > array[i]) {
            is_sorted = false;
            break;
        }
    }

    if (rank < num_p - 1) {
        MPI_Send(&last_element, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }

    if (rank > 0) {
        MPI_Recv(&next_first_element, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        if (next_first_element > array[0]) {
            is_sorted = false;
        }
    }

    bool global_is_sorted;
    MPI_Reduce(&is_sorted, &global_is_sorted, 1, MPI_C_BOOL, MPI_LAND, 0,
               MPI_COMM_WORLD);

    if (rank == 0) {
        if (global_is_sorted) {
            printf("Global array is sorted.\n");
        } else {
            printf("Global array is not sorted.\n");
        }
    }
}