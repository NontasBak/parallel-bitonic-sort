#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "include/bitonic.h"

void generateRandomNumbers(int *array, int num_q, int rank) {
    srand(time(NULL) + rank);
    for (int i = 0; i < num_q; i++) {
        array[i] = rand() % 100;
    }
}

int main(int argc, char **argv) {
    int num_p, num_q;
    int rank, size;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    num_p = size; // Default is 4 (check the Makefile)
    num_q = pow(2, 5);

    int *array = (int *)malloc(num_q * sizeof(int));
    generateRandomNumbers(array, num_q, rank);

    // printf("Process %d: ", rank);
    // for (int i = 0; i < num_q; i++) {
    //     printf("%d ", array[i]);
    // }
    // printf("\n");

    start_time = MPI_Wtime();

    bitonicSort(rank, num_p, num_q, &array);

    end_time = MPI_Wtime();

    // Print final result after sorting
    //print(rank, size, num_q, array);
    
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        printf("Execution time: %f seconds\n", end_time - start_time);
    }

    free(array);
    MPI_Finalize();
    

    return 0;
}