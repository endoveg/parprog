#include <cmath>
#include <mpi.h>
#include "Inner.h"

void Inner::Init(int isize, int jsize) {
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    this->isize = isize;
    this->jsize = jsize;
    int iMax = localIUpperBound();
    int jMax = localJUpperBound();
    aLoc = MallocOrDie(iMax, jMax);
    aBuf = MallocOrDie(1, 3);

    for (int i=0; i<iMax; i++)
        for (int j=0; j<jMax; j++)
            aLoc[i][j] = 10 * localItoI(i) + localJtoJ(j);

    if (rank != 0)
        MPI_Send((void*) aLoc[0], 3, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
    if (rank != world_size - 1) {
        MPI_Recv((void *) aBuf[0], 3, MPI_DOUBLE, rank + 1, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void Inner::Calculate() {
    int iMax = localIUpperBound();
    int jMax = localJUpperBound();
    for (int i=8; i < iMax; i++) {
        for (int j = 0; j < jMax - 3; j++)
            aLoc[i][j] = sin(0.00001 * aLoc[i - 8][j + 3]);
        if (rank != world_size - 1)
            for (int j = jMax - 3; j < jMax; j++)
                aLoc[i][j] = sin(0.00001 * aBuf[0][3 - (jMax - j)]);
        if (rank != 0)
            MPI_Send((void*) aLoc[i-7], 3, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
        if (rank != world_size - 1) {
            MPI_Recv((void *) aBuf[0], 3, MPI_DOUBLE, rank + 1, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
}

void Inner::Print(std::ostream &out) {
    double** rowBuf = MallocOrDie(1, jsize);
    // fuck checks
    int *recvcount = static_cast<int *>(malloc(sizeof(int) * world_size));
    int *displs = static_cast<int *>(malloc (sizeof (int) * world_size));
    // init recvcount
    int true_rank = rank;
    for (int p = 0; p < world_size; p++) {
        // dirty hack
        this->rank = p;
        recvcount[p] = localJUpperBound();
    }
    // restore true rank
    this->rank = true_rank;
    // init displs
    displs[0] = 0;
    for (int p = 1; p < world_size; p++)
        displs[p] = displs[p - 1] + recvcount[p - 1];
    for (int i = 0; i < isize; i++) {
        MPI_Gatherv(aLoc[i], localJUpperBound(), MPI_DOUBLE, rowBuf[0],
                recvcount, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            for (int k = 0; k < jsize; k++)
                out << rowBuf[0][k] << ' ';
            out << '\n';
        }
    }
}

void Inner::Gather() {
    double **rowBuf = MallocOrDie(1, jsize);
    // fuck checks
    int *recvcount = static_cast<int *>(malloc(sizeof(int) * world_size));
    int *displs = static_cast<int *>(malloc(sizeof(int) * world_size));
    // init recvcount
    int true_rank = rank;
    for (int p = 0; p < world_size; p++) {
        // dirty hack
        this->rank = p;
        recvcount[p] = localJUpperBound();
    }
    // restore true rank
    this->rank = true_rank;
    // init displs
    displs[0] = 0;
    for (int p = 1; p < world_size; p++)
        displs[p] = displs[p - 1] + recvcount[p - 1];
    for (int i = 0; i < isize; i++) {
        MPI_Gatherv(aLoc[i], localJUpperBound(), MPI_DOUBLE, rowBuf[0],
                    recvcount, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
}

int Inner::localItoI(int i) {
    return i;
}

int Inner::localJtoJ(int j) {
    int chunk_size = ceil(static_cast<double> (jsize) / world_size);
    return j + chunk_size*rank;
}

int Inner::localIUpperBound() {
    return isize;
}

int Inner::localJUpperBound() {
    int chunk_size = ceil(static_cast<double> (jsize) / world_size);
    int r = jsize % chunk_size;
    if ((rank == world_size-1) && r)
        return r;
    else
        return chunk_size;
}
