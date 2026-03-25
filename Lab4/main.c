#define LAB4_EXTEND
 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include "Lab4_IO.h"
#include "timer.h"
 
#define EPSILON           0.00001
#define DAMPING_FACTOR    0.85
#define CONVERGENCEVAR     10      // check convergence
 
int main(int argc, char *argv[]) {
   int rank, size;
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Comm_size(MPI_COMM_WORLD, &size);
 

   // read nodecount and partition                                      
   int nodecount;
   FILE *ip;
   if ((ip = fopen("data_input_meta", "r")) == NULL) {
       printf("Process %d: Error opening data_input_meta.\n", rank);
       MPI_Abort(MPI_COMM_WORLD, 253);
   }
   fscanf(ip, "%d\n", &nodecount);
   fclose(ip);
 
   int base  = nodecount / size;
   int extra = nodecount % size;
 
   int *recvcounts = malloc(size * sizeof(int));
   int *displs     = malloc(size * sizeof(int));
   for (int p = 0; p < size; ++p) {
       int ps = p * base + (p < extra ? p : extra);
       int pe = ps + base + (p < extra ? 1 : 0);
       recvcounts[p] = pe - ps;
       displs[p]     = ps;
   }
 
   int start   = displs[rank];
   int localN = recvcounts[rank];
 
   // load data
   struct node *nodehead;
   if (node_init(&nodehead, start, start + localN) != 0) {
       printf("Process %d: node_init failed.\n", rank);
       MPI_Abort(MPI_COMM_WORLD, 254);
   }
 

   // Allocate and initialise (p = r * N)
   
   double *rContrib     = malloc(nodecount * sizeof(double));
   double *localContrib = malloc(localN   * sizeof(double));
   double *localp       = malloc(localN   * sizeof(double));
   double *localPPrev  = malloc(localN   * sizeof(double)); // for checking convergence
   double *localPNew   = malloc(localN   * sizeof(double));
 
   for (int i = 0; i < localN; ++i) {
       localp[i]       = 1.0;
       localContrib[i] = 1.0 / (double)nodehead[i].num_out_links;
   }
   MPI_Allgatherv(localContrib, localN, MPI_DOUBLE, rContrib, recvcounts, displs, MPI_DOUBLE, MPI_COMM_WORLD);
 
   const double teleport = 1.0 - DAMPING_FACTOR;
 

   // computation

   int iterationcount = 0;
   double t_start, t_end;
   double localErr[2], globalErr[2];
   int converged = 0;
 
   MPI_Barrier(MPI_COMM_WORLD);
   GET_TIME(t_start);
 
   while (!converged) {
 
       // save p  at the start of each check
       for (int i = 0; i < localN; ++i) {
           localPPrev[i] = localp[i];
        }

       // run convergence interval iterations before checking
       for (int step = 0; step < CONVERGENCEVAR; ++step) {
           ++iterationcount;
 
           for (int i = 0; i < localN; ++i) {
               double sum = 0.0;
               int  nin = nodehead[i].num_in_links;
               int *inl = nodehead[i].inlinks;

               for (int k = 0; k < nin; ++k) {
                   sum += rContrib[inl[k]];
                }
 
               double p_new     = teleport + DAMPING_FACTOR * sum;
               localPNew[i]   = p_new;
               localContrib[i] = p_new / (double)nodehead[i].num_out_links;
           }
 
           // gather up
           MPI_Allgatherv(localContrib, localN, MPI_DOUBLE, rContrib, recvcounts, displs, MPI_DOUBLE, MPI_COMM_WORLD);
 
           // swap p's
           double *tmp = localp;
           localp     = localPNew;
           localPNew = tmp;
       }
 
       // check convergence once per convergence interval
       localErr[0] = 0.0;
       localErr[1] = 0.0;
       for (int i = 0; i < localN; ++i) {
           double diff = localp[i] - localPPrev[i];
           localErr[0] += diff * diff;
           localErr[1] += localPPrev[i] * localPPrev[i];
       }
       MPI_Allreduce(localErr, globalErr, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
 
       // scale epsilon by convergence interval
       if (sqrt(globalErr[0] / globalErr[1]) < EPSILON * CONVERGENCEVAR)
           converged = 1;
   }
 


   MPI_Barrier(MPI_COMM_WORLD);
   GET_TIME(t_end);
 

   // recover final output r = p/N
   
   double inv_N = 1.0 / nodecount;
   for (int i = 0; i < localN; ++i) {
       localp[i] *= inv_N;
    }

   double *r_full = NULL;
   if (rank == 0) {
       r_full = malloc(nodecount * sizeof(double));
    }
 
   MPI_Gatherv(localp, localN, MPI_DOUBLE, r_full, recvcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
 
   if (rank == 0) {
       Lab4_saveoutput(r_full, nodecount, t_end - t_start);
       printf("Converged in %d iterations. Time: %f s\n", iterationcount, t_end - t_start);
       free(r_full);
   }
 

   // destroy and free
   node_destroy(nodehead, localN);
   free(rContrib); free(localContrib);
   free(localp); free(localPPrev); free(localPNew);
   free(recvcounts); free(displs);
 
   MPI_Finalize();
   return 0;
}

