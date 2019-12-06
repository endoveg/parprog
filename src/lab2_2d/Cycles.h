#include <iostream>
#include <mpi.h>
#ifndef LAB2D_CYCLES_H
#define LAB2D_CYCLES_H



class Cycles {
public:
    virtual void Init(int isize, int jsize) = 0 ;
    virtual void Calculate() = 0;
    virtual void Print(std::ostream &out) = 0;
    virtual void Gather() = 0; //used to imitate time spent on gathering a[i][j] in order to print it
};

double** MallocOrDie(int m, int n);
void Clean(double **aBuf, int m, int n);


#endif //LAB2D_CYCLES_H
