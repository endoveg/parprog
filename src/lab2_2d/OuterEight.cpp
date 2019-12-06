#include "OuterEight.h"
#include "Cycles.h"
#include <mpi.h>
#include <cmath>
#include <iomanip>

void OuterEight::Init(int isize, int jsize) {
    // a[4*j+rank][:]
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    this->isize = isize;
    this->jsize = jsize;

    int iMax = localIUpperBound();
    int jMax = localJUpperBound();
    aLoc = MallocOrDie(iMax, jMax);
    aBuf = nullptr; // as no buffer is needed

    for (int i=0; i<iMax; i++)
        for (int j=0; j<jMax; j++)
            aLoc[i][j] = 10 * localItoI(i) + localJtoJ(j);
}

int OuterEight::localItoI(int i) {
    return 8*i + rank;
}

int OuterEight::localJtoJ(int j) {
    return j;
}

int OuterEight::localIUpperBound() {
    if (rank + 1 <= (isize % 8))
        return floor(isize / 8.0) + 1;
    else
        return floor(isize / 8.0);
}

int OuterEight::localJUpperBound() {
    return jsize;
}

void OuterEight::Print(std::ostream &out) {
    // JUSTGATHER -> simulate Send / Recv additional cost
    out << std::setprecision(7);
    double *rowBuf;
    if (( rowBuf = (double*) malloc(sizeof(double)*jsize)) == NULL) {
        printf("Memory allocation error\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int iMax = localIUpperBound();
    if (rank != 0) {
        for (int i = 0; i < iMax; i++)
            MPI_Send((void *) aLoc[i], jsize, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
         for (int i = 0; i < isize; i++) {
             if (i % 8 == 0) {
                for (int j = 0; j < jsize; j++)
                    out <<  (aLoc[i / 8][j]) << " ";
                out << "\n";
             } else {
                 MPI_Recv((void *) rowBuf, jsize, MPI_DOUBLE, i % 8, 0,
                          MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                 for (int j = 0; j < jsize; j++)
                     out << rowBuf[j] << " ";
                 out << "\n";
             }
         }

    }
}

void OuterEight::Calculate() {
    int iMax = localIUpperBound();
    int jMax = localJUpperBound();
    // if local_i = 2, than i = 8
    // -8 -> -2 because jobs for four processors interleave
    for (int i=1; i < iMax; i++)
        for (int j=0; j < jMax - 3; j++)
            aLoc[i][j] = sin(0.00001*aLoc[i-1][j+3]);
}

void OuterEight::Gather() {
    double *rowBuf;
    if ((rowBuf = (double *) malloc(sizeof(double) * jsize)) == NULL) {
        printf("Memory allocation error\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int iMax = localIUpperBound();
    if (rank != 0) {
        for (int i = 0; i < iMax; i++)
            MPI_Send((void *) aLoc[i], jsize, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        for (int i = 0; i < isize; i++) {
            if (i % 8 == 0) {
            } else {
                MPI_Recv((void *) rowBuf, jsize, MPI_DOUBLE, i % 8, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
}
