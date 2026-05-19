#pragma once
#include "SORSolver.h"

// Метод Зейделя - это МВР c w = 1
class SeidelSolver : public SORSolver {
public:
    SeidelSolver() : SORSolver(1.0) {}
};