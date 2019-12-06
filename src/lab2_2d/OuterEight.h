#include "Cycles.h"
#ifndef LAB2D_OUTEREIGHT_H
#define LAB2D_OUTEREIGHT_H

class OuterEight: public Cycles {
public:
    void Init(int isize, int jsize) override;
    void Calculate() override;
    void Print(std::ostream &out) override;
    void Gather() override;
private:
    int localItoI(int i);
    int localJtoJ(int j);
    int localIUpperBound();
    int localJUpperBound();
    int isize, jsize;
    double** aLoc;
    double** aBuf;
    int rank;
    int world_size;
};

#endif
