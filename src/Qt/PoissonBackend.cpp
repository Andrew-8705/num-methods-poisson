#include "Qt/PoissonBackend.h"
#include "Solvers/SORSolver.h"

#include <algorithm>
#include <cmath>
#include <QColor>

Field2D PoissonBackend::exactSolution(const TestProblem& prob, int n, int m) {
    Field2D field;
    field.n = n;
    field.m = m;
    field.a = prob.a();
    field.b = prob.b();
    field.c = prob.c();
    field.d = prob.d();
    field.values.resize((n + 1) * (m + 1));

    double h = (field.b - field.a) / n;
    double k = (field.d - field.c) / m;
    for (int i = 0; i <= n; ++i) {
        for (int j = 0; j <= m; ++j) {
            double x = field.a + i * h;
            double y = field.c + j * k;
            field.values[i * (m + 1) + j] = prob.exact_u(x, y);
        }
    }
    return field;
}

SolverResult PoissonBackend::solveTestProblem(int n, int m, double eps_max, int max_iter) {
    TestProblem testProb;
    SORSolver solver(1.0);
    return solver.solve(testProb, n, m, eps_max, max_iter);
}

Field2D PoissonBackend::fieldFromGrid(const Grid& grid) {
    Field2D field;
    field.n = grid.n;
    field.m = grid.m;
    field.a = grid.a;
    field.b = grid.b;
    field.c = grid.c;
    field.d = grid.d;
    field.values.resize((grid.n + 1) * (grid.m + 1));

    for (int i = 0; i <= grid.n; ++i) {
        for (int j = 0; j <= grid.m; ++j) {
            field.values[i * (grid.m + 1) + j] = grid.data[i][j];
        }
    }
    return field;
}

static void findMinMax(const Field2D& field, double& minValue, double& maxValue) {
    minValue = field.values.empty() ? 0.0 : field.values[0];
    maxValue = minValue;
    for (double value : field.values) {
        if (value < minValue) minValue = value;
        if (value > maxValue) maxValue = value;
    }
}

static QRgb mapValueToColor(double value, double minValue, double maxValue) {
    if (maxValue <= minValue) {
        return qRgb(128, 128, 255);
    }
    double t = (value - minValue) / (maxValue - minValue);
    int r = static_cast<int>(std::clamp(255.0 * t, 0.0, 255.0));
    int b = static_cast<int>(std::clamp(255.0 * (1.0 - t), 0.0, 255.0));
    int g = static_cast<int>(std::clamp(128.0 * (1.0 - std::abs(2.0 * t - 1.0)), 0.0, 255.0));
    return qRgb(r, g, b);
}

double PoissonBackend::calculateGlobalError(const Field2D& exact, const Field2D& numeric) {
    if (exact.n != numeric.n || exact.m != numeric.m) {
        return 0.0;
    }

    double maxError = 0.0;
    for (int i = 0; i <= exact.n; ++i) {
        for (int j = 0; j <= exact.m; ++j) {
            double err = std::abs(exact.at(i, j) - numeric.at(i, j));
            if (err > maxError) {
                maxError = err;
            }
        }
    }
    return maxError;
}
