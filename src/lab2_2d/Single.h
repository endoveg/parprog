#ifndef LAB2D_SINGLE_H
#define LAB2D_SINGLE_H


#include "Cycles.h"

class Single: public Cycles {
public:
    void Init(int isize, int jsize) override;
    void Calculate() override;
    void Print(std::ostream &out) override;
    void Gather() override;
private:
    int isize, jsize;
    double** aLoc;
};


#endif //LAB2D_SINGLE_H
