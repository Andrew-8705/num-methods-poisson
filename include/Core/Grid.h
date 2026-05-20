#pragma once
#include <vector>

struct Grid {
    int n, m;
    double h, k;
    double a, b, c, d;
    std::vector<std::vector<double>> data;

    Grid(int n, int m, double a, double b, double c, double d) 
        : n(n), m(m), a(a), b(b), c(c), d(d) 
    {
        h = (b - a) / n;
        k = (d - c) / m;
        data.resize(n + 1, std::vector<double>(m + 1, 0.0));
    }
};

struct SolverResult {
    Grid grid;
    int iterations;
    double achieved_eps;
    double max_residual = 0.0;
};