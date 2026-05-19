#pragma once
#include "Core/Solver.h"

class SORSolver : public Solver {
protected:
    double w; // Параметр релаксации

public:
    explicit SORSolver(double omega);

    SolverResult solve(const Problem& prob, int n, int m, double eps_max, int max_iter) override;
};