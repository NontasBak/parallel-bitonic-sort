#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/bitonic.h"

void bitonicSort(int rank, int num_p, int num_q, int **array)
{
    // Initial sorting of the processes
    sortProcesses(rank, num_q, *array);

    // step defines the amount of blocks to be recursively merged up or down.
    // For example step = 2 means that 2 blocks will be merged up *and* 2 blocks
    // merged down (except for the last loop).
    // i defines the start id of the block to be merged.
    for (int step = 2; step <= num_p; step *= 2)
    {
        for (int i = 0; i < num_p; i += step * 2)
        {
            // printf("womp! %d %d\n", step, i);
            if (i <= rank && rank < i + step)
            {
                merge_up(rank, i, step, num_q, *array);
            }
            if (step + i < num_p && step + i <= rank &&
                rank < 2 * step + i)
            { // check if we're at the last loop
                merge_down(rank, step + i, step, num_q, *array);
            }
        }
        // printf("womp! %d\n", step);
        MPI_Barrier(MPI_COMM_WORLD);
        elbowMerge(rank, num_p, num_q, step, array);
    }
}

// size is the amount of blocks to be merged
void merge_up(int rank, int start_id, int size, int num_q, int *array)
{
    int cut = size / 2;

    // Create a sub-communicator for merge_up (only the processes that are part
    // of the merge)
    MPI_Comm merge_up_comm;
    int color =
        (rank >= start_id && rank < start_id + size) ? 1 : MPI_UNDEFINED;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &merge_up_comm);

    if (color == 1)
    {
        // Compare element wise the blocks
        for (int i = 0; i < cut; i++)
        {
            compare_up_elementwise(rank, start_id + i, start_id + cut + i,
                                   num_q, array);
        }
        if (cut == 1)
        {
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

void merge_down(int rank, int start_id, int size, int num_q, int *array)
{
    int cut = size / 2;

    // Create a sub-communicator for merge_down (only the processes that are
    // part of the merge)
    MPI_Comm merge_down_comm;
    int color =
        (rank >= start_id && rank < start_id + size) ? 1 : MPI_UNDEFINED;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &merge_down_comm);

    if (color == 1)
    {
        // Compare element wise the blocks
        for (int i = 0; i < cut; i++)
        {
            compare_down_elementwise(rank, start_id + i, start_id + cut + i,
                                     num_q, array);
        }
        if (cut == 1)
        {
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
void compare_up_elementwise(int rank, int rank_p1, int rank_p2, int num_q, int *array)
{
    if (rank == rank_p1)
    {
        int *recv_array = (int *)malloc(num_q * sizeof(int));
        MPI_Request send_request, recv_request;

        MPI_Isend(array, num_q, MPI_INT, rank_p2, 0, MPI_COMM_WORLD, &send_request);
        MPI_Irecv(recv_array, num_q, MPI_INT, rank_p2, 0, MPI_COMM_WORLD, &recv_request);

        MPI_Wait(&send_request, MPI_STATUS_IGNORE);
        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);

        keepMinElements(array, recv_array, num_q);
        free(recv_array);
    }
    else if (rank == rank_p2)
    {
        int *recv_array = (int *)malloc(num_q * sizeof(int));
        MPI_Request send_request, recv_request;

        MPI_Irecv(recv_array, num_q, MPI_INT, rank_p1, 0, MPI_COMM_WORLD, &recv_request);
        MPI_Isend(array, num_q, MPI_INT, rank_p1, 0, MPI_COMM_WORLD, &send_request);

        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
        MPI_Wait(&send_request, MPI_STATUS_IGNORE);

        keepMaxElements(array, recv_array, num_q);
        free(recv_array);
    }
}

void compare_down_elementwise(int rank, int rank_p1, int rank_p2, int num_q, int *array)
{
    if (rank == rank_p1)
    {
        int *recv_array = (int *)malloc(num_q * sizeof(int));
        MPI_Request send_request, recv_request;

        MPI_Isend(array, num_q, MPI_INT, rank_p2, 0, MPI_COMM_WORLD, &send_request);
        MPI_Irecv(recv_array, num_q, MPI_INT, rank_p2, 0, MPI_COMM_WORLD, &recv_request);

        MPI_Wait(&send_request, MPI_STATUS_IGNORE);
        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);

        keepMaxElements(array, recv_array, num_q);
        free(recv_array);
    }
    else if (rank == rank_p2)
    {
        int *recv_array = (int *)malloc(num_q * sizeof(int));
        MPI_Request send_request, recv_request;

        MPI_Irecv(recv_array, num_q, MPI_INT, rank_p1, 0, MPI_COMM_WORLD, &recv_request);
        MPI_Isend(array, num_q, MPI_INT, rank_p1, 0, MPI_COMM_WORLD, &send_request);

        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
        MPI_Wait(&send_request, MPI_STATUS_IGNORE);

        keepMinElements(array, recv_array, num_q);
        free(recv_array);
    }
}

void sortProcesses(int rank, int num_q, int *array)
{
    // Even processes are ascending sorted
    // Odd processes are descending sorted
    if (rank % 2 == 0)
    {
        qsort(array, num_q, sizeof(int), compare_asc);
    }
    else
    {
        qsort(array, num_q, sizeof(int), compare_des);
    }
}

void keepMinElements(int *array, int *recv_array, int num_q)
{
    for (int i = 0; i < num_q; i++)
    {
        if (array[i] > recv_array[i])
        {
            array[i] = recv_array[i];
        }
    }
}

void keepMaxElements(int *array, int *recv_array, int num_q)
{
    for (int i = 0; i < num_q; i++)
    {
        if (array[i] < recv_array[i])
        {
            array[i] = recv_array[i];
        }
    }
}

void elbowMerge(int rank, int num_p, int num_q, int size, int **array)
{
    int elbow; // index of elbow
    findElbow(num_q, *array, &elbow);
    // sorting placing min elbow element first
    sortElbow(num_q, elbow, rank, size, array);
}

// finds the min element as elbow
void findElbow(int num_q, int *array, int *elbow)
{
    int min = 0; // first element as elbow

    for (int i = 1; i < num_q; i++)
    {
        if (array[min] > array[i]) // elements smaller than elbow (descending order)
        {
            min = i;
            /* MAYBE FASTER?
            //in descending order if next element from elbow is greater
            //than elbow then elbow is the min of the bitonic
            if(array[min]<array[min+1] && min<= num_q-2){
                break;
            }
            */
        }
    }
    (*elbow) = min;
}

void sortElbow(int num_q, int elbow, int rank, int size, int **array)
{
    int left, right; // array pointers

    if (elbow == 0) // first element as elbow
    {
        left = num_q - 1; // left pointer to the end
        right = elbow + 1;
    }
    else if (elbow == num_q - 1) // last element as elbow
    {
        right = 0; // right pointer to the start
        left = elbow - 1;
    }
    else
    {
        left = elbow - 1;
        right = elbow + 1;
    }

    // memory allocation for sorted array
    int *sorted = (int *)malloc(num_q * sizeof(int));
    if (sorted == NULL)
    {
        printf("Error by allocating memory!");
        return;
    }

    if (rank % (size * 2) < size) // defines which processes should be in ascending or descending order
    {
        // sort process in ascending order
        int index = 0;
        sorted[index] = (*array)[elbow]; // first element as elbow

        while (left != right) // elbow sorting
        {
            index++;
            compareElements(&left, &right, index, num_q, *array, sorted);
        }
        sorted[index + 1] = (*array)[left];
    }

    else
    {
        // sort in descending order
        int index = num_q - 1;
        sorted[index] = (*array)[elbow]; // last element as elbow

        while (left != right) // elbow sort
        {
            index--;
            compareElements(&left, &right, index, num_q, *array, sorted);
        }
        sorted[index - 1] = (*array)[left];
    }

    free(*array);    // delete input array
    *array = sorted; // array pointer to sorted array
}

void compareElements(int *left, int *right, int index, int num_q, int *array, int *sorted)
{
    int l = (*left);
    int r = (*right);

    if (array[r] <= array[l])
    {
        sorted[index] = array[r];
        (*right)++;
    }
    else
    {
        sorted[index] = array[l];
        (*left)--;
    }

    if ((*left) < 0) // when left pointer reaches the start of array
    {                // place it at the and of the array
        (*left) = num_q - 1;
    }
    else if ((*right) > num_q - 1) // when right pointer reaches the end of the array
    {                              // place it at the start of the array
        (*right) = 0;
    }
}

// helpers for qsort
int compare_asc(const void *a, const void *b) { return (*(int *)a - *(int *)b); }
int compare_des(const void *a, const void *b) { return (*(int *)b - *(int *)a); }

void print(int rank, int size, int num_q, int *array)
{
    MPI_Barrier(MPI_COMM_WORLD);
    for (int i = 0; i < size; i++)
    {
        if (rank == i)
        {
            printf("Process %d: ", rank);
            for (int j = 0; j < num_q; j++)
            {
                printf("%2d ", array[j]);
            }
            MPI_Barrier(MPI_COMM_WORLD);
            printf("\n");
            fflush(stdout);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}