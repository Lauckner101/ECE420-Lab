#include <corecrt_search.h>
#include <stdio.h>
#include <stdlib.h>
#include "lab1_IO.h"
#include "timer.h"
#include <pthread.h>
#include <math.h>

int **matA;
int **matB;
int **output;
int num_threads;
int size;

void* matrix_multiplication(void* arg) {

    return NULL;

}

// int** matrix_multiplication(int *size) {
//     int **matA;

//     int **matB;

//     int **output;

//     int out = Lab1_loadinput(&matA, &matB, size);

//     output = malloc(*size * sizeof(int*));
//     for (int i = 0; i < *size; i++) {
//         output[i] = malloc(*size * sizeof(int));
//     }

//     for (int i=0; i < *size; i++) {
//         for (int j=0; j < *size; j++) {
//             int sum = 0;

//             for (int n=0; n < *size; n++) {
//                 sum += matA[i][n] * matB[n][j];
//             }

//             output[i][j] = sum;
//         }
//     }


//     return output;

// }

int main (int argc, char* argv[]) {
    double start_time, end_time;

    if (argc != 2) {
        printf("Invalid number of arguments\n");
        return EXIT_FAILURE;
    }
    
    num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        printf(stderr, "Number of threads must be greater than 0\n");
        return EXIT_FAILURE;
    } else if (sqrt(num_threads) != (int)sqrt(num_threads)) {
        printf(stderr,"Number of threads must be a perfect square\n");
        return EXIT_FAILURE;
    }


    int out = Lab1_loadinput(&matA, &matB, &size);
    if (out != 0) {
        printf(stderr, "Error loading input\n");
        return EXIT_FAILURE;
    }

    if ((size * size) % num_threads != 0) {
        fprintf(stderr, "Matrix size² must be divisble by num of threads\n");
        return EXIT_FAILURE;
    }

    output = malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        output[i] = malloc(size * sizeof(int));
    }

    GET_TIME(start_time);
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, matrix_multiplication, (void*)i);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    GET_TIME(end_time);
    printf("Time taken: %f seconds\n", end_time - start_time);

    Lab1_saveoutput(output, &size, end_time - start_time);
    return EXIT_SUCCESS;
}