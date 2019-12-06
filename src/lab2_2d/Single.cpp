#include "Single.h"
#include "Cycles.h"
#include <cmath>
#include <iomanip>

void Single::Init(int isize, int jsize) {
    this->isize = isize;
    this->jsize = jsize;

    aLoc = MallocOrDie(isize, jsize);

    for (int i=0; i<isize; i++)
        for (int j=0; j<jsize; j++)
            aLoc[i][j] = 10 * i + j;
}

void Single::Print(std::ostream &out) {
    out << std::setprecision(7);
    for (int i=0; i < isize; i++) {
        for (int j = 0; j < jsize; j++)
            out << aLoc[i][j] << " ";
        out << "\n";
    }
}

void Single::Gather() {};

void Single::Calculate() {
    for (int i=8; i < isize; i++)
        for (int j=0; j < jsize - 3; j++)
            aLoc[i][j] = sin(0.00001*aLoc[i-8][j+3]);
}
