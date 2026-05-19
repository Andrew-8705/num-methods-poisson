#pragma once
#include "Grid.h"
#include "Problem.h"

class Solver {
public:
    virtual ~Solver() = default;

    // Метод solve принимает любой Problem 
    virtual SolverResult solve(const Problem& prob, int n, int m, double eps_max, int max_iter) = 0;
};