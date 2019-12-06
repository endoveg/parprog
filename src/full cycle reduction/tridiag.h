#ifndef PARPROG_TRIDIAG_H
#define PARPROG_TRIDIAG_H
#include <vector>


using namespace std;
class tridiag {
public:
    tridiag(int N, double b);
    tridiag(int N, vector<double>& A, vector<double>& B, vector<double>& C, vector<double>& RHS);
    vector<double> Solve() const;
    vector<double> parSolve();
private:
    /*
     * v'' = -alpha(x)*v + alpha
     * y0 = (b-1)*x + 1
     * y approx. y0 + v
     * y'' = exp(-y)
     */
    double alpha(double x);
    vector<double> A, B, C;
    vector<double> RHS;
    double b;
    int N;
    double h;
};

vector<double> cycleReduction(int, vector<double>&, vector<double>&, vector<double>&, vector<double>&);


#endif //PARPROG_TRIDIAG_H
