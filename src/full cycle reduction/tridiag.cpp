#include "tridiag.h"
#include <cmath>

tridiag::tridiag(int N, double b) : N(N), b(b) {
    h = 1.0 / (N - 1.0);
    A = vector<double> (N);
    B = vector<double> (N);
    C = vector<double> (N);
    RHS = vector<double> (N);
    /*
     * v(0) = 0
     * v(1) = 0
     */
    A[0] = 0; B[0] = 1; C[0] = 0; RHS[0] = 0;
    A[N-1] = 0; B[N-1] = 1; C[N-1] = 0; RHS[N-1] = 0;
    const double h2 = h*h;
    /*
     * 1/h**2 * (ym+1 - 2*ym + ym-1) = fm + 1/12 * (fm+1 -2*fm + fm-1)
     * fm = -alpha(xm)*ym + alpha(xm)
     */
    for (int i = 1; i < N - 1; i++) {
        A[i] = 1/(h2) + alpha(h*(i-1)) / 12.0;
        B[i] = -2/(h2) + alpha(h*i) - 2.0/12.0 * alpha(h*i);
        C[i] = 1/(h2) + alpha(h*(i+1)) / 12.0;
        RHS[i] = alpha(h*i) + 1.0 / 12.0 * (alpha(h*(i+1)) - 2*alpha(h*i) + alpha(h*(i-1)));
    }
}

inline double tridiag::alpha(double x) {
    return exp( -((b-1)*x + 1) );
}

vector<double> tridiag::Solve() const{
    vector<double> v(N);
    vector<double> c_star(N, 0.0);
    vector<double> RHS_star(N, 0.0);
    // forward sweep
    c_star[0] = C[0] / B[0];
    RHS_star[0] = RHS[0] / B[0];
    for (int i = 1; i < N; i++) {
        double m = 1.0 / (B[i] - A[i] * c_star[i-1]);
        c_star[i] = C[i] * m;
        RHS_star[i] = (RHS[i] - A[i] * RHS_star[i-1]) * m;
    }
    // backward sweep
    v[N-1] = RHS_star[N-1];
    for (int i = N-2; i >= 0; i--) {
        v[i] = RHS_star[i] - c_star[i] * v[i+1];
    }
    return v;
}

vector<double> tridiag::parSolve() {
    return cycleReduction(N, A, B, C, RHS);
}

vector<double> cycleReduction(int N, vector<double> &A, vector<double> &B, vector<double> &C, vector<double>& RHS) {
    // N = 2**m - 1, so 2 is not possible
    if (N == 1) {
        return vector<double> {RHS[0] / B[0]};
    } else {
        vector<double> A_reduced(N / 2), B_reduced(N / 2), C_reduced(N / 2), RHS_reduced(N / 2);
        // see https://algowiki-project.org/ru/Полный_метод_циклической_редукции
        // preprocces A, B, C, RHS for even indices
        double __B_i_inv;
        {
#pragma omp parallel
#pragma omp for private(__B_i_inv)
            for (int i = 0; i < N; i += 2) {
                // division is expensive operation, let's store 1.0 / B[i]
                __B_i_inv = 1.0 / B[i];
                A[i] *= __B_i_inv;
                B[i] = 1.0;
                C[i] *= __B_i_inv;
                RHS[i] *= __B_i_inv;
            }
            // prepare coefficients for reduced system

            // treat second and penult equations separately
            // second eqution (index 1)
            // C[2k] / B[2k] and A[2k] / B[2k] are already computed in previous section
            // C[2k] / B[2k] -> C[2k]
            // A[2k] / B[2k] -> C[2k]
#pragma omp single
            {
                A_reduced[0] = 0;
                C_reduced[0] = -C[1] * C[2];
                B_reduced[0] = B[1] - A[1] * C[0] - C[1] * A[2];
                RHS_reduced[0] = RHS[1] - A[1] * RHS[0] - C[1] * RHS[2];
            }

            // penult equation (index n - 2)
            // N != 2 never
#pragma omp single
            {
                A_reduced[(N - 2) / 2] = -A[N - 2] * A[N - 3];
                C_reduced[(N - 2) / 2] = 0;
                B_reduced[(N - 2) / 2] = B[N - 2] - A[N - 2] * C[N - 3] - C[N - 2] * A[N - 1];
                RHS_reduced[(N - 2) / 2] = RHS[N - 2] - A[N - 2] * RHS[N - 3] - C[N - 2] * RHS[N - 1];
            }
#pragma omp for
            for (int i=3; i < N - 2; i+=2) {
                A_reduced[i/2] = -A[i]*A[i-1];
                C_reduced[i/2] = -C[i]*C[i+1];
                B_reduced[i/2] = B[i] - A[i]*C[i-1] - C[i]*A[i+1];
                RHS_reduced[i/2] = RHS[i] - A[i]*RHS[i-1] - C[i]*RHS[i+1];
            }
        }
        const auto x_reduced = cycleReduction((N-1)/2, A_reduced, B_reduced, C_reduced, RHS_reduced);
        auto x = vector<double> (N);
        // reverse
        {
            int index;
#pragma omp parallel private(index)
#pragma omp for
            for (int i = 1; i < N - 1; i++) {
		index = i / 2;
		if (i % 2)
		    x[i] = x_reduced[index];
		else
                    x[i] = RHS[i] - A[i] * x_reduced[index-1] - C[i] * x_reduced[index];
            }
        }
	x[0] = RHS[0] - C[0] * x[1];
	x[N - 1] = RHS[N - 1] - A[N - 1] * x[N - 2];
        return x;
    }
}
