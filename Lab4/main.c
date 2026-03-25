#define LAB4_EXTEND

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include "Lab4_IO.h"
#include "timer.h"

#define EPSILON        0.00001
#define DAMPING_FACTOR 0.85
#define CONVERGENCEVAR 10       // check convergence every N iterations

int main(int argc, char *argv[]) {
    // instantiate variables
    struct node *nodehead;
    int nodecount;
    double *r, *r_pre;
    int i, j;
    int iterationcount;
    double start, end;
    FILE *ip;
    /* INSTANTIATE MORE VARIABLES IF NECESSARY */
    int rank, size;
    int *recvcounts, *displs;
    int localN, startNode;
    double *localContrib, *localPNew;
    int converged;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // load data
    if ((ip = fopen("data_input_meta", "r")) == NULL) {
        if (rank == 0) printf("Error opening the data_input_meta file.\n");
        MPI_Abort(MPI_COMM_WORLD, 253);
    }
    fscanf(ip, "%d\n", &nodecount);
    fclose(ip);

    int base  = nodecount / size;
    int extra = nodecount % size;

    recvcounts = malloc(size * sizeof(int));
    displs     = malloc(size * sizeof(int));
    for (int p = 0; p < size; ++p) {
        int ps = p * base + (p < extra ? p : extra);
        int pe = ps + base + (p < extra ? 1 : 0);
        recvcounts[p] = pe - ps;
        displs[p]     = ps;
    }
    startNode = displs[rank];
    localN    = recvcounts[rank];

    if (node_init(&nodehead, startNode, startNode + localN)) {
        MPI_Abort(MPI_COMM_WORLD, 254);
    }

    r     = malloc(nodecount * sizeof(double));
    r_pre = malloc(localN   * sizeof(double));

    localContrib = malloc(localN * sizeof(double));
    localPNew    = malloc(localN * sizeof(double));

    double *localp = malloc(localN * sizeof(double));
    iterationcount = 0;
    for (i = 0; i < localN; ++i) {
        localp[i]      = 1.0;
        localContrib[i] = 1.0 / (double)nodehead[i].num_out_links;
    }

    MPI_Allgatherv(localContrib, localN, MPI_DOUBLE,
                   r, recvcounts, displs, MPI_DOUBLE, MPI_COMM_WORLD);

    const double teleport = 1.0 - DAMPING_FACTOR;
    double localErr[2], globalErr[2];
    converged = 0;

    MPI_Barrier(MPI_COMM_WORLD);
    GET_TIME(start);
    
    // core calculation
    do {
        for (i = 0; i < localN; ++i)
            r_pre[i] = localp[i];

        for (j = 0; j < CONVERGENCEVAR; ++j) {
            ++iterationcount;

            for (i = 0; i < localN; ++i) {
                double sum = 0.0;
                int  nin = nodehead[i].num_in_links;
                int *inl = nodehead[i].inlinks;
                int  k;
                for (k = 0; k < nin; ++k)
                    sum += r[inl[k]];

                double p_new    = teleport + DAMPING_FACTOR * sum;
                localPNew[i]   = p_new;
                localContrib[i] = p_new / (double)nodehead[i].num_out_links;
            }

            MPI_Allgatherv(localContrib, localN, MPI_DOUBLE,
                           r, recvcounts, displs, MPI_DOUBLE, MPI_COMM_WORLD);

            double *tmp = localp;
            localp      = localPNew;
            localPNew   = tmp;
        }

        localErr[0] = 0.0;
        localErr[1] = 0.0;
        for (i = 0; i < localN; ++i) {
            double diff = localp[i] - r_pre[i];
            localErr[0] += diff * diff;
            localErr[1] += r_pre[i] * r_pre[i];
        }
        MPI_Allreduce(localErr, globalErr, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        if (sqrt(globalErr[0] / globalErr[1]) < EPSILON * CONVERGENCEVAR)
            converged = 1;

    } while (!converged);

    MPI_Barrier(MPI_COMM_WORLD);
    GET_TIME(end);

    double inv_N = 1.0 / nodecount;
    for (i = 0; i < localN; ++i)
        localp[i] *= inv_N;

    double *r_full = NULL;
    if (rank == 0)
        r_full = malloc(nodecount * sizeof(double));

    MPI_Gatherv(localp, localN, MPI_DOUBLE,
                r_full, recvcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        Lab4_saveoutput(r_full, nodecount, end - start);
        printf("Converged in %d iterations. Time: %f s\n", iterationcount, end - start);
    }

    // post processing
    node_destroy(nodehead, localN);
    free(r); free(r_pre);
    free(localp); free(localContrib); free(localPNew);
    free(recvcounts); free(displs);
    if (rank == 0) free(r_full);

    MPI_Finalize();
    return 0;
}