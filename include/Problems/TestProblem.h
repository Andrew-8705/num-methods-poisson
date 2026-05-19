#pragma once
#include "Core/Problem.h"

class TestProblem : public Problem {
public:
    double a() const override;
    double b() const override;
    double c() const override;
    double d() const override;

    double f(double x, double y) const override;

    double mu1(double y) const override;
    double mu2(double y) const override;
    double mu3(double x) const override;
    double mu4(double x) const override;

    bool has_exact_solution() const override;
    double exact_u(double x, double y) const override;
};