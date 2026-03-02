#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "Lab3IO.h"
#include "timer.h"

int thread_count;

int main(int argc, char* argv[]) {
    int i, j, k, size;
    double** A;
    double* B;
    double temp;
    int* index;
    double start, end;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number of threads>\n", argv[0]);
        exit(1);
    }

    thread_count = atoi(argv[1]);
    if (thread_count <= 0) {
        fprintf(stderr, "Thread count must be at least 1\n");
        exit(1);
    }

    Lab3LoadInput(&A, &size);

    B = CreateVec(size);
    index = (int*)malloc(size * sizeof(int));

    for (i = 0; i < size; i++)
        index[i] = i;

    GET_TIME(start);

    if (size == 1) {
        B[0] = A[0][1] / A[0][0];
    } else {

        #pragma omp parallel num_threads(thread_count) default(none) \
            shared(A, B, index, size) private(i, j, k, temp)
        {
            // Gaussian elimination
            for (k = 0; k < size - 1; k++) {

                #pragma omp single
                {
                    int kp = k;
                    double maxval = fabs(A[index[k]][k]);

                    for (int r = k + 1; r < size; r++) {
                        double v = fabs(A[index[r]][k]);
                        if (v > maxval) {
                            maxval = v;
                            kp = r;
                        }
                    }

                    if (kp != k) {
                        int tmp = index[k];
                        index[k] = index[kp];
                        index[kp] = tmp;
                    }
                }

                #pragma omp for schedule(static)
                for (i = k + 1; i < size; i++) {
                    temp = A[index[i]][k] / A[index[k]][k];
                    for (j = k; j < size + 1; ++j) {
                        A[index[i]][j] -= A[index[k]][j] * temp;
                    }
                }
            }

            // Jordan elimination
            for (k = size - 1; k > 0; k--) {
                #pragma omp for schedule(static)
                for (i = 0; i < k; i++) {
                    temp = A[index[i]][k] / A[index[k]][k];
                    A[index[i]][k] -= temp * A[index[k]][k];
                    A[index[i]][size] -= temp * A[index[k]][size];
                }
            }

            // Output
            #pragma omp for schedule(static)
            for (k = 0; k < size; k++) {
                B[k] = A[index[k]][size] / A[index[k]][k];
            }
        }
    }

    GET_TIME(end);

    printf("%lf\n", end - start);
    Lab3SaveOutput(B, size, end - start);

    DestroyVec(B);
    DestroyMat(A, size);
    free(index);
    return 0;
}