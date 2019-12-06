#include "OuterTwo.h"
#include "Cycles.h"
#include <mpi.h>
#include <cmath>
#include <iomanip>

void OuterTwo::Init(int isize, int jsize) {
    // a[2*j][:]
    // a[2*j+1][:]
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

int OuterTwo::localItoI(int i) {
    return 2*i + rank;
}

int OuterTwo::localJtoJ(int j) {
    return j;
}

int OuterTwo::localIUpperBound() {
    if ((isize % 2) == 0)
        return isize / 2;
    else
        return ceil(isize / 2.0) + rank;
}

int OuterTwo::localJUpperBound() {
    return jsize;
}

void OuterTwo::Print(std::ostream &out) {
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
             if (i % 2 == 0) {
                for (int j = 0; j < jsize; j++)
                    out <<  (aLoc[i / 2][j]) << " ";
                out << "\n";
             } else {
                 MPI_Recv((void *) rowBuf, jsize, MPI_DOUBLE, 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                 for (int j = 0; j < jsize; j++)
                     out << rowBuf[j] << " ";
                 out << "\n";
             }
         }

    }
}

void OuterTwo::Calculate() {
    int iMax = localIUpperBound();
    int jMax = localJUpperBound();
    // if local_i = 4, than i = 8
    // -8 -> -4 because jobs for two processors interleave
    for (int i=4; i < iMax; i++)
        for (int j=0; j < jMax - 3; j++)
            aLoc[i][j] = sin(0.00001*aLoc[i-4][j+3]);
}

void OuterTwo::Gather() {
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
            if (i % 2 == 0) {
            } else {
                MPI_Recv((void *) rowBuf, jsize, MPI_DOUBLE, 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
}
