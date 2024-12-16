#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/bitonic.h"

void bitonicSort(int rank, int num_p, int num_q, int* array) {
    // Initial sorting of the processes
    sortProcesses(rank, num_p, num_q, 1, array);

    // step defines the amount of blocks to be recursively merged up or down.
    // For example step = 2 means that 2 blocks will be merged up *and* 2 blocks
    // merged down (except for the last loop).
    // i defines the start id of the block to be merged.
    for (int step = 2; step <= num_p; step *= 2) {
        for (int i = 0; i < num_p; i += step * 2) {
            // printf("womp! %d %d\n", step, i);
            if (i <= rank && rank < i + step) {
                merge_up(rank, i, step, num_q, array);
            }
            if (step + i < num_p && step + i <= rank &&
                rank < 2 * step + i) {  // check if we're at the last loop
                merge_down(rank, step + i, step, num_q, array);
            }
        }
        // printf("womp! %d\n", step);
        MPI_Barrier(MPI_COMM_WORLD);

        sortProcesses(rank, num_p, num_q, step, array);
    }
}

// size is the amount of blocks to be merged
void merge_up(int rank, int start_id, int size, int num_q, int* array) {
    int cut = size / 2;

    // Create a sub-communicator for merge_up (only the processes that are part
    // of the merge)
    MPI_Comm merge_up_comm;
    int color =
        (rank >= start_id && rank < start_id + size) ? 1 : MPI_UNDEFINED;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &merge_up_comm);

    if (color == 1) {
        // Compare element wise the blocks
        for (int i = 0; i < cut; i++) {
            compare_up_elementwise(rank, start_id + i, start_id + cut + i,
                                   num_q, array);
        }
        if (cut == 1) {
            MPI_Barrier(merge_up_comm);
            MPI_Comm_free(&merge_up_comm);
            return;
        }
        MPI_Barrier(merge_up_comm);

        // Recursion
        merge_up(rank, start_id, cut, num_q, array);
        merge_up(rank, start_id + cut, cut, num_q, array);

        MPI_Comm_free(&merge_up_comm);
    }
}

void merge_down(int rank, int start_id, int size, int num_q, int* array) {
    int cut = size / 2;

    // Create a sub-communicator for merge_down (only the processes that are
    // part of the merge)
    MPI_Comm merge_down_comm;
    int color =
        (rank >= start_id && rank < start_id + size) ? 1 : MPI_UNDEFINED;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &merge_down_comm);

    if (color == 1) {
        // Compare element wise the blocks
        for (int i = 0; i < cut; i++) {
            compare_down_elementwise(rank, start_id + i, start_id + cut + i,
                                     num_q, array);
        }
        if (cut == 1) {
            MPI_Barrier(merge_down_comm);
            MPI_Comm_free(&merge_down_comm);
            return;
        }
        MPI_Barrier(merge_down_comm);

        // Recursion
        merge_down(rank, start_id, cut, num_q, array);
        merge_down(rank, start_id + cut, cut, num_q, array);

        MPI_Comm_free(&merge_down_comm);
    }
}

// rank_p1 keeps the min elements, rank_p2 keeps the max elements
void compare_up_elementwise(int rank, int rank_p1, int rank_p2, int num_q, int* array) {
    if (rank == rank_p1) {
        int* recv_array = (int*)malloc(num_q * sizeof(int));
        MPI_Request send_request, recv_request;

        MPI_Isend(array, num_q, MPI_INT, rank_p2, 0, MPI_COMM_WORLD, &send_request);
        MPI_Irecv(recv_array, num_q, MPI_INT, rank_p2, 0, MPI_COMM_WORLD, &recv_request);

        MPI_Wait(&send_request, MPI_STATUS_IGNORE);
        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);

        keepMinElements(array, recv_array, num_q);
        free(recv_array);
    } else if (rank == rank_p2) {
        int* recv_array = (int*)malloc(num_q * sizeof(int));
        MPI_Request send_request, recv_request;

        MPI_Irecv(recv_array, num_q, MPI_INT, rank_p1, 0, MPI_COMM_WORLD, &recv_request);
        MPI_Isend(array, num_q, MPI_INT, rank_p1, 0, MPI_COMM_WORLD, &send_request);

        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
        MPI_Wait(&send_request, MPI_STATUS_IGNORE);

        keepMaxElements(array, recv_array, num_q);
        free(recv_array);
    }
}

void compare_down_elementwise(int rank, int rank_p1, int rank_p2, int num_q, int* array) {
    if (rank == rank_p1) {
        int* recv_array = (int*)malloc(num_q * sizeof(int));
        MPI_Request send_request, recv_request;

        MPI_Isend(array, num_q, MPI_INT, rank_p2, 0, MPI_COMM_WORLD, &send_request);
        MPI_Irecv(recv_array, num_q, MPI_INT, rank_p2, 0, MPI_COMM_WORLD, &recv_request);

        MPI_Wait(&send_request, MPI_STATUS_IGNORE);
        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);

        keepMaxElements(array, recv_array, num_q);
        free(recv_array);
    } else if (rank == rank_p2) {
        int* recv_array = (int*)malloc(num_q * sizeof(int));
        MPI_Request send_request, recv_request;

        MPI_Irecv(recv_array, num_q, MPI_INT, rank_p1, 0, MPI_COMM_WORLD, &recv_request);
        MPI_Isend(array, num_q, MPI_INT, rank_p1, 0, MPI_COMM_WORLD, &send_request);

        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
        MPI_Wait(&send_request, MPI_STATUS_IGNORE);

        keepMinElements(array, recv_array, num_q);
        free(recv_array);
    }
}

// Size is the amount of blocks to be sorted increasingly or decreasingly in
// turns. For example size = 2 means that 2 blocks will be sorted increasingly,
// 2 blocks will be sorted decreasingly etc.
void sortProcesses(int rank, int num_p, int num_q, int size, int* array) {
    // This function should implement the "Elbow merge" sorting method.
    // The process should be a bitonic.
    // The sorting algorithm is not implemented at the moment.
    if (rank % (size * 2) < size) {
        qsort(array, num_q, sizeof(int), compare_asc);
    } else {
        qsort(array, num_q, sizeof(int), compare_des);
    }

}

void keepMinElements(int* array, int* recv_array, int num_q) {
    for (int i = 0; i < num_q; i++) {
        if (array[i] > recv_array[i]) {
            array[i] = recv_array[i];
        }
    }
}

void keepMaxElements(int* array, int* recv_array, int num_q) {
    for (int i = 0; i < num_q; i++) {
        if (array[i] < recv_array[i]) {
            array[i] = recv_array[i];
        }
    }
}
