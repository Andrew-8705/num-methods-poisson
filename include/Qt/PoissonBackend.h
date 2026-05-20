#pragma once

#include "Core/Grid.h"
#include "Core/Solver.h"
#include "Problems/TestProblem.h"

#include <vector>

struct Field2D {
    int n;
    int m;
    double a;
    double b;
    double c;
    double d;
    std::vector<double> values;

    Field2D() : n(0), m(0), a(0), b(0), c(0), d(0) {}

    double at(int i, int j) const {
        return values[i * (m + 1) + j];
    }
};

class PoissonBackend {
public:
    static Field2D exactSolution(const TestProblem& prob, int n, int m);
    static SolverResult solveTestProblem(int n, int m, double eps_max, int max_iter, double omega);
    static SolverResult solveMainProblem(int n, int m, double eps_max, int max_iter, double omega);
    static double calculateOptimalOmega(int n, int m, double a, double b, double c, double d);
    static Field2D fieldFromGrid(const Grid& grid);
    static double calculateGlobalError(const Field2D& exact, const Field2D& numeric);
};
