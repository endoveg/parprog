#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdlib.h>

unsigned long localIToI(int world_size, int rank, unsigned long isize, unsigned long i) {
    unsigned long chunk_size = ceil((double) isize / world_size);
    return i + chunk_size*rank;
}

unsigned long localIUpperBound(int world_size, int rank, unsigned isize) {
    unsigned long chunk_size = ceil((double) isize / world_size);
    unsigned long r = isize % chunk_size;
    if ((rank == world_size-1) && r)
        return r;
    else
        return chunk_size;
}

int main(int argc, char* argv[]) {
#ifdef PRINT
    FILE *ff;
    ff = fopen("resultp.txt", "w");
#endif
    double start, end;
    // 0. read isize, jsize
    unsigned long isize, jsize;
    if (argc != 3) {
        printf("./lab2 isize jsize\n");
        return 1;
    }
    if (! (isize = strtoul(argv[1], NULL, 10))) {
        printf("Unable to convert isize to unsigned long\n");
        return 1;
    }
    if (! (jsize = strtoul(argv[2], NULL, 10))) {
        printf("Unable to convert jsize to unsigned long\n");
        return 1;
    }

    // 1. init world
    if (MPI_Init(&argc, &argv) ) {
        printf("Error: MPI_Init\n");
        return 1;
    }
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    start = MPI_Wtime();

    /*
     * 2. init a[i][j]
     *  parallelize over i
     */
    unsigned long upperBound = localIUpperBound(world_size, rank, isize);
    double** aLoc;
    double** aBuf;
    if ((aBuf = (double**) malloc(sizeof(double)*4)) == NULL) {
        printf("Memory allocation error\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if ((aLoc = (double**) malloc(sizeof(double)*upperBound)) == NULL) {
        printf("Memory allocation error\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    // 2.1 compute aLoc
    for (unsigned long iLoc=0; iLoc < upperBound; iLoc++) {
        if ((aLoc[iLoc] = (double *) malloc(sizeof(double) * jsize)) == NULL) {
            printf("Memory allocation error\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        } else {
            for (unsigned long jLoc = 0; jLoc < jsize; jLoc++)
                aLoc[iLoc][jLoc] = 10 * localIToI(world_size, rank, isize, iLoc) + jLoc;
        }
    }
    // assert ceil(isize/world_size) >= 4
    // 2.1 recieve and send 4 overlapping layers
    for (int i=0; i < 4; i++) {
        if ((aBuf[i] = (double *) malloc(sizeof(double) * jsize)) == NULL) {
            printf("Memory allocation error\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    if (rank != world_size - 1)
        for (int i=0; i < 4; i++)
            MPI_Recv((void*) aBuf[i], jsize, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if (rank != 0)
        for (int i=0; i < 4; i++)
            MPI_Send((void*) aLoc[i], jsize, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD);
    /*
     * 3. calculation itself
     */
    for (unsigned long iLoc=0; iLoc < upperBound-4; iLoc++)
        for (unsigned long jLoc=5; jLoc < jsize; jLoc++)
            aLoc[iLoc][jLoc] = sin(0.00001*aLoc[iLoc+4][jLoc-5]);

    if (rank != world_size - 1)
        for (int i=0; i < 4; i++)
            for (unsigned long jLoc=5; jLoc < jsize; jLoc++)
                aLoc[upperBound-4+i][jLoc] = sin(0.00001*aBuf[i][jLoc-5]);
#ifdef PRINT
     //4. let 0 print all that stuff
    if (rank != 0)
        for (unsigned long iLoc = 0; iLoc < upperBound; iLoc++)
            MPI_Send((void *) aLoc[iLoc], jsize, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    else {
        for (unsigned long iLoc = 0; iLoc < upperBound; iLoc++) {
            for (unsigned long jLoc = 0; jLoc < jsize; jLoc++)
                fprintf(ff, "%f ", aLoc[iLoc][jLoc]);
            fprintf(ff, "\n");
        }
        for (int r = 1; r < world_size; r++) {
            unsigned long int upperBoundR = localIUpperBound(world_size, r, isize);
            for (unsigned long iLoc = 0; iLoc < upperBoundR; iLoc++)
                MPI_Recv((void *) aLoc[iLoc], jsize, MPI_DOUBLE, r, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (unsigned long iLoc = 0; iLoc < upperBoundR; iLoc++) {
                for (unsigned long jLoc = 0; jLoc < jsize; jLoc++)
                    fprintf(ff, "%f ", aLoc[iLoc][jLoc]);
                fprintf(ff, "\n");
            }
        }
    }
    fclose(ff);
#endif
	if (rank == 0) {
	    end = MPI_Wtime();
	    printf("%d %f\n", world_size, end - start);
	}
    MPI_Finalize();
    return 0;
}
