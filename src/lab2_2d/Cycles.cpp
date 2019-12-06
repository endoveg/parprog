#include <cstdlib>
#include <mpi.h>
#include <cstdio>

double** MallocOrDie(int m, int n) {
    double** p;
    if ((p = (double**) malloc(sizeof(double*)*m)) == NULL) {
        printf("Memory allocation error\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    for (int i=0; i<m; i++) {
        if ((p[i] = (double*) malloc(sizeof(double)*n)) == NULL) {
            printf("Memory allocation error\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    return p;
}
