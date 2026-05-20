#include "Solvers/SORSolver.h"
#include <cmath>
#include <algorithm>
#include <vector>

SORSolver::SORSolver(double omega) : w(omega) {}

SolverResult SORSolver::solve(const Problem& prob, int n, int m, double eps_max, int max_iter) {
    Grid grid(n, m, prob.a(), prob.b(), prob.c(), prob.d());
    
    double h2_inv = 1.0 / (grid.h * grid.h);
    double k2_inv = 1.0 / (grid.k * grid.k);
    double A = 2.0 * (h2_inv + k2_inv); 

    // Инициализация границ
    for (int i = 0; i <= n; ++i) {
        double x = prob.a() + i * grid.h;
        grid.data[i][0] = prob.mu3(x); 
        grid.data[i][m] = prob.mu4(x); 
    }
    for (int j = 0; j <= m; ++j) {
        double y = prob.c() + j * grid.k;
        grid.data[0][j] = prob.mu1(y); 
        grid.data[n][j] = prob.mu2(y); 
    }

    // Предварительно вычисляем правую часть f(x,y), чтобы не вызывать её миллионы раз в цикле
    std::vector<std::vector<double>> F(n + 1, std::vector<double>(m + 1, 0.0));
    for (int i = 1; i < n; ++i) {
        double x = prob.a() + i * grid.h;
        for (int j = 1; j < m; ++j) {
            double y = prob.c() + j * grid.k;
            F[i][j] = prob.f(x, y);
        }
    }

    // Начальное приближение (линейная интерполяция)
    for (int j = 1; j < m; ++j) {
        for (int i = 1; i < n; ++i) {
            double x = prob.a() + i * grid.h;
            grid.data[i][j] = grid.data[0][j] + 
                              ((x - prob.a()) / (prob.b() - prob.a())) * (grid.data[n][j] - grid.data[0][j]);
        }
    }

    int iter = 0;
    double current_eps = eps_max + 1.0;

    // Оптимизируем кэш-память: i делаем внешним циклом, чтобы чтение шло последовательно по памяти
    while (iter < max_iter && current_eps > eps_max) {
        current_eps = 0.0;

        for (int i = 1; i < n; ++i) {
            for (int j = 1; j < m; ++j) {
                double v_old = grid.data[i][j];

                // Теперь здесь нет тяжелых вызовов функций, только арифметика
                double v_seidel = ( (grid.data[i-1][j] + grid.data[i+1][j]) * h2_inv + 
                                    (grid.data[i][j-1] + grid.data[i][j+1]) * k2_inv + 
                                    F[i][j] ) / A;

                double v_new = (1.0 - w) * v_old + w * v_seidel;

                double diff = std::abs(v_new - v_old);
                if (diff > current_eps) {
                    current_eps = diff;
                }

                grid.data[i][j] = v_new;
            }
        }
        iter++;
    }

    return {grid, iter, current_eps};
}