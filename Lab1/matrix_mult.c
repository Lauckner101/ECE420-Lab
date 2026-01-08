#include <stdio.h>
#include <stdlib.h>
#include "lab1_IO.h"


int** matrix_multiplication() {
    int **matA;

    int **matB;

    int **output;

    int size;


    int output = Lab1_loadinput(&matA, &matB, &size);



    for (int i=0; i < size; i++) {
        for (int j=0; j < size; j++) {
            int sum = 0;

            for (int n=0; n < size; n++) {
                sum += matA[i][n] * matB[n][j];
            }

            output[i][j] = sum;
        }
    }


    return output;

}




int main (int argc, char* argv[]) {
    int **output = matrix_multiplication();

    for (int i=0; i < 10; i++) {
        for (int j=0; j < 10; j++) {
            printf("%d ", output[i][j]);
        }
        printf("\n");
    }

}