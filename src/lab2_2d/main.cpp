#include <iostream>
#include <fstream>
#include <mpi.h>
#include "OuterTwo.h"
#include "Single.h"
#include "OuterFour.h"
#include "OuterEight.h"
#include "Inner.h"

Cycles& ChooseMethod(int p) {
    if (p == 1) {
        static Single s{};
        return s;
    } else if (p == 2) {
        static OuterTwo o2{};
        return o2;
    } else if (p == 4) {
        static OuterFour o4{};
        return o4;
    } else if (p == 8) {
        static OuterEight o8{};
        return o8;
    } else {
        static Inner i;
        return i;
    }
}

int main(int argc, char* argv[]) {

    // some c boredom
    int isize, jsize;
    if (argc != 3) {
        printf("./2d isize jsize\n");
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
    if (MPI_Init(NULL, NULL) ) {
        printf("Error: MPI_Init\n");
        return 1;
    }

    double start, end;
    int rank, world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    Cycles& cycle = ChooseMethod(world_size);
    start = MPI_Wtime();
    cycle.Init(isize, jsize);
    cycle.Calculate();
#ifdef PRINT
    std::ofstream out;
    out.open("results.txt");
    cycle.Print(out);
    out.close();
#else
    cycle.Gather();
#endif
    if (rank == 0) {
        end = MPI_Wtime();
        printf("%d %f\n", world_size, end - start);
    }
    MPI_Finalize();
    return 0;
}
