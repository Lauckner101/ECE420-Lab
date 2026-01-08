#include <stdio.h>
#include <stdlib.h>
#include "lab1_IO.h"


int** matrix_multiplication(int *size) {
    int **matA;

    int **matB;

    int **output;

    int out = Lab1_loadinput(&matA, &matB, size);

    output = malloc(*size * sizeof(int*));
    for (int i = 0; i < *size; i++) {
        output[i] = malloc(*size * sizeof(int));
    }

    for (int i=0; i < *size; i++) {
        for (int j=0; j < *size; j++) {
            int sum = 0;

            for (int n=0; n < *size; n++) {
                sum += matA[i][n] * matB[n][j];
            }

            output[i][j] = sum;
        }
    }


    return output;

}




int main (int argc, char* argv[]) {
    int size;
    int **output = matrix_multiplication(&size);

    for (int i=0; i < size; i++) {
        for (int j=0; j < size; j++) {
            printf("%d ", output[i][j]);
        }
        printf("\n");
    }

}