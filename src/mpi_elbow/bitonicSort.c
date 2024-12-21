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

            MPI_Barrier(MPI_COMM_WORLD);
        }
        elbowMerge(rank, num_p, num_q, group_size, array, sort_descending);
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

void elbowMerge(int rank, int num_p, int num_q, int size, int **array,
                bool sort_descending) {
    int elbow = findElbow(num_q, *array);  // index of elbow
    // sorting placing min elbow element first
    sortElbow(num_q, elbow, rank, size, array, sort_descending);
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

void sortElbow(int num_q, int elbow, int rank, int size, int **array,
               bool sort_descending) {
    int left, right;  // array pointers

    if (elbow == 0)  // first element as elbow
    {
        left = num_q - 1;  // left pointer to the end
        right = elbow + 1;
    } else if (elbow == num_q - 1)  // last element as elbow
    {
        right = 0;  // right pointer to the start
        left = elbow - 1;
    } else {
        left = elbow - 1;
        right = elbow + 1;
    }

    // memory allocation for sorted array
    int *sorted = (int *)malloc(num_q * sizeof(int));
    if (sorted == NULL) {
        printf("Error by allocating memory!");
        return;
    }

    if (!sort_descending)  // defines which processes should be in
                           // ascending or descending order
    {
        // sort process in ascending order
        int index = 0;
        sorted[index] = (*array)[elbow];  // first element as elbow

        while (left != right)  // elbow sorting
        {
            index++;
            compareElements(&left, &right, index, num_q, *array, sorted);
        }
        sorted[index + 1] = (*array)[left];
    }

    else {
        // sort in descending order
        int index = num_q - 1;
        sorted[index] = (*array)[elbow];  // last element as elbow

        while (left != right)  // elbow sort
        {
            index--;
            compareElements(&left, &right, index, num_q, *array, sorted);
        }
        sorted[index - 1] = (*array)[left];
    }

    free(*array);     // delete input array
    *array = sorted;  // array pointer to sorted array
}

void compareElements(int *left, int *right, int index, int num_q, int *array,
                     int *sorted) {
    int l = (*left);
    int r = (*right);

    if (array[r] <= array[l]) {
        sorted[index] = array[r];
        (*right)++;
    } else {
        sorted[index] = array[l];
        (*left)--;
    }

    if ((*left) < 0)  // when left pointer reaches the start of array
    {                 // place it at the and of the array
        (*left) = num_q - 1;
    } else if ((*right) >
               num_q - 1)  // when right pointer reaches the end of the array
    {                      // place it at the start of the array
        (*right) = 0;
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