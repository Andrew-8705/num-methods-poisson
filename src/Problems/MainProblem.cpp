#include "Problems/MainProblem.h"
#include <cmath>
#include <numbers>

double MainProblem::a() const { return 0.0; }
double MainProblem::b() const { return 3.0; }
double MainProblem::c() const { return 0.0; }
double MainProblem::d() const { return 1.0; }

double MainProblem::f(double x, double y) const {
    return std::cosh(x - y);
}

double MainProblem::mu1(double y) const {
    double temp = std::sin(std::numbers::pi * y);
    return temp * temp; // sin^2(pi*y)
}

double MainProblem::mu2(double y) const {
    return 0.0;
}

double MainProblem::mu3(double x) const {
    return std::cosh(x * x - 3 * x) - 1.0;
}

double MainProblem::mu4(double x) const {
    return 0.0;
}