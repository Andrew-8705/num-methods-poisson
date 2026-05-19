#include "Problems/TestProblem.h"
#include <cmath>

double TestProblem::a() const { return 0.0; }
double TestProblem::b() const { return 3.0; }
double TestProblem::c() const { return 0.0; }
double TestProblem::d() const { return 1.0; }

bool TestProblem::has_exact_solution() const { return true; }

// u* = sin^2(x * y^2)
double TestProblem::exact_u(double x, double y) const {
    double temp = std::sin(x * y * y);
    return temp * temp;
}

// f*(x,y) = - (u_xx + u_yy)
double TestProblem::f(double x, double y) const {
    double t = 2 * x * y * y;
    double u_xx = 2 * std::pow(y, 4) * std::cos(t);
    double u_yy = 2 * x * std::sin(t) + 8 * x * x * y * y * std::cos(t);
    return -(u_xx + u_yy);
}

double TestProblem::mu1(double y) const { return exact_u(a(), y); }
double TestProblem::mu2(double y) const { return exact_u(b(), y); }
double TestProblem::mu3(double x) const { return exact_u(x, c()); }
double TestProblem::mu4(double x) const { return exact_u(x, d()); }