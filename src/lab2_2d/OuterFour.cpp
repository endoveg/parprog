#include "OuterFour.h"
#include "Cycles.h"
#include <mpi.h>
#include <cmath>
#include <iomanip>

void OuterFour::Init(int isize, int jsize) {
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

int OuterFour::localItoI(int i) {
    return 4*i + rank;
}

int OuterFour::localJtoJ(int j) {
    return j;
}

int OuterFour::localIUpperBound() {
    if (rank + 1 <= (isize % 4))
        return floor(isize / 4.0) + 1;
    else
        return floor(isize / 4.0);
}

int OuterFour::localJUpperBound() {
    return jsize;
}

void OuterFour::Print(std::ostream &out) {
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
             if (i % 4 == 0) {
                for (int j = 0; j < jsize; j++)
                    out <<  (aLoc[i / 4][j]) << " ";
                out << "\n";
             } else {
                 MPI_Recv((void *) rowBuf, jsize, MPI_DOUBLE, i % 4, 0,
                          MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                 for (int j = 0; j < jsize; j++)
                     out << rowBuf[j] << " ";
                 out << "\n";
             }
         }

    }
}

void OuterFour::Calculate() {
    int iMax = localIUpperBound();
    int jMax = localJUpperBound();
    // if local_i = 2, than i = 8
    // -8 -> -2 because jobs for four processors interleave
    for (int i=2; i < iMax; i++)
        for (int j=0; j < jMax - 3; j++)
            aLoc[i][j] = sin(0.00001*aLoc[i-2][j+3]);
}

void OuterFour::Gather() {
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
            if (i % 4 == 0) {
            } else {
                MPI_Recv((void *) rowBuf, jsize, MPI_DOUBLE, i % 4, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
}
