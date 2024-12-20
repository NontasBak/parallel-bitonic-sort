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
        MPI_Barrier(MPI_COMM_WORLD);
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
    bool sort = true, steady = true;
    int elbow = 0; // index of elbow

    findElbow(&sort, &steady, num_q, *array, &elbow);
    MPI_Barrier(MPI_COMM_WORLD);
    // sorting placing elbow element first
    sortElbow(sort, steady, num_q, elbow, rank, size, array);
}

// sort is true when bitonic is ascending first, false for descenfing first
// steady is true when bitonic is steady (contains the same number)
// returns the place of elbow, elbow is 0 when array is sorted all the way or steady
void findElbow(bool *sort, bool *steady, int num_q, int *array, int *elbow)
{
    bool noSort = true; // no sorting has been found

    for (int i = 0; i < num_q - 1; i++)
    {
        if (array[i] < array[i + 1])
        {
            if (noSort)
            {                   // initiallize as ascending
                (*sort) = true; // ascending
                (*steady) = false;
                noSort = false;
            }
            else
            {
                if ((*sort) == false)
                {
                    (*elbow) = i;
                    return;
                }
            }
        }
        else if (array[i] > array[i + 1])
        {

            if (noSort)
            {                    // initiallize as descending
                (*sort) = false; // descending
                (*steady) = false;
                noSort = false;
            }
            else
            {
                if ((*sort) == true)
                {
                    (*elbow) = i;
                    return;
                }
            }
        }
    }
    if (noSort) // If no sort has been found it is a steady array
    {
        (*steady) = true;
        (*elbow) = 0;
    }
    else
    {
        // when sorting is found but no elbow
        // sequence is already sorted
        (*elbow) = 0;
    }
}

void sortElbow(bool sort, bool steady, int num_q, int elbow, int rank, int size, int **arr)
{

    int *array = (*arr);

    // steady array is sorted
    if (steady)
    {
        return;
    }

    int left = elbow - 1;  // left pointer to array
    int right = elbow + 1; // right pointer to array

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
        if (sort == true && elbow == 0) // already sorted in ascending order
        {
            free(sorted);
            return;
        }
        else if (sort == false && elbow == 0) // already sorted in descending order
        {
            // change sorting order
            reverse(array, num_q, sorted);
            free(*arr);
            *arr = sorted;
            return;
        }
        else if (sort == false) // descending first
        {
            // elbow is the min element
            int index = 0;
            sorted[index] = array[elbow]; // first element as elbow

            while (left != right) // elbow sorting
            {
                index++;
                circular_compare(&left, &right, index, sort, array, sorted, num_q);
            }
            sorted[index + 1] = array[left];
        }
        else // ascending first
        {
            // elbow is the max element
            int index = num_q - 1;
            sorted[index] = array[elbow]; // last element as elbow

            while (left != right) // elbow sort
            {
                index--;
                circular_compare(&left, &right, index, sort, array, sorted, num_q);
            }
            sorted[index - 1] = array[left];
        }
    }
    else
    {
        // sort in descending order
        if (sort == false && elbow == 0) // already sorted in descending order
        {
            free(sorted);
            return;
        }
        else if (sort == true && elbow == 0) // already sorted in ascending order
        {
            // change sorting order
            reverse(array, num_q, sorted);
            free(*arr);
            *arr = sorted;
            return;
        }
        else if (sort == false) // descending first
        {
            // elbow is the min element
            int index = num_q - 1;
            sorted[index] = array[elbow]; // last element as elbow

            while (left != right) // elbow sort
            {
                index--;
                circular_compare(&left, &right, index, sort, array, sorted, num_q);
            }
            sorted[index - 1] = array[left];
        }
        else // ascending first
        {
            // elbow is the max element
            int index = 0;
            sorted[index] = array[elbow]; // first element as elbow

            while (left != right) // elbow sort
            {
                index++;
                circular_compare(&left, &right, index, sort, array, sorted, num_q);
            }
            sorted[index + 1] = array[left];
        }
    }

    free(*arr);    // delete input array
    *arr = sorted; // array pointer to sorted array
}

// finds min when bitonic is ascending first
//or max element when bitonic is depending first
void circular_compare(int *left, int *right, int index, bool sort,
                      int *array, int *sorted, int num_q)
{
    if (sort == false) // descending first
    {                  // find min element
        if (array[(*right)] <= array[(*left)])
        {
            sorted[index] = array[(*right)];
            (*right)++;
        }
        else
        {
            sorted[index] = array[(*left)];
            (*left)--;
        }
    }
    else // ascending first
    {
        // find max element
        if (array[(*right)] <= array[(*left)])
        {
            sorted[index] = array[(*left)];
            (*left)--;
        }
        else
        {
            sorted[index] = array[(*right)];
            (*right)++;
        }
    }

    // pointers scan the array in a circular manner
    if ((*left) < 0) // when left pointer reaches the start of array
    {                // place it at the and of the array
        (*left) = num_q - 1;
    }
    else if ((*right) > num_q - 1) // when right pointer reaches the end of the array
    {                              // place it at the start of the array
        (*right) = 0;
    }
}

//changes the sorting order of sorted array
void reverse(int *array, int num_q, int *sorted)
{
    int j = num_q - 1;
    for (int i = 0; i < num_q; i++)
    {
        sorted[j] = array[i];
        j--;
    }
}

//helpers for qsort
int compare_asc(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
int compare_des(const void* a, const void* b) { return (*(int*)b - *(int*)a); }


void print(int rank, int size, int num_q, int* array) {
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