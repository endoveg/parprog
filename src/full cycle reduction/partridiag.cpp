#include <omp.h>
#include <iostream>
#include <iomanip>
#include "tridiag.h"


using namespace std;
int main(int argc, char* argv[]) {
    const int N = 2048 - 1;
    const vector<double> B = {.1, .2, .3, .4, .5, .6, .7, .8, .9, 1};
    cout << "x " << "v(x) " << "b " << endl;
    for (const auto& b: B) {
        tridiag sys(N, b);
        const auto sol = sys.parSolve();
        for (int j=0; j < N; j++)
            cout << static_cast<double> (j) / (N - 1) << " " << sol[j] << " " << b << " " << endl;
    }
    return 0;
}

