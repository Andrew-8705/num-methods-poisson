#include "Solvers/SORSolver.h"
#include <cmath>
#include <algorithm>

SORSolver::SORSolver(double omega) : w(omega) {}

SolverResult SORSolver::solve(const Problem& prob, int n, int m, double eps_max, int max_iter) {
    Grid grid(n, m, prob.a(), prob.b(), prob.c(), prob.d());
    
    double h2_inv = 1.0 / (grid.h * grid.h);
    double k2_inv = 1.0 / (grid.k * grid.k);
    
    double A = 2.0 * (h2_inv + k2_inv); 

    // Инициализация границ
    for (int i = 0; i <= n; ++i) {
        double x = prob.a() + i * grid.h;
        grid.data[i][0] = prob.mu3(x); // Нижняя граница
        grid.data[i][m] = prob.mu4(x); // Верхняя граница
    }
    for (int j = 0; j <= m; ++j) {
        double y = prob.c() + j * grid.k;
        grid.data[0][j] = prob.mu1(y); // Левая граница
        grid.data[n][j] = prob.mu2(y); // Правая граница
    }

    // Начальное приближение
    for (int j = 1; j < m; ++j) {
        for (int i = 1; i < n; ++i) {
            double x = prob.a() + i * grid.h;
            // V(x,y) = V_left + (x - a)/(b - a) * (V_right - V_left)
            grid.data[i][j] = grid.data[0][j] + 
                              ((x - prob.a()) / (prob.b() - prob.a())) * 
                              (grid.data[n][j] - grid.data[0][j]);
        }
    }

    // Главный итерационный цикл
    int iter = 0;
    double current_eps = eps_max + 1.0;

    while (iter < max_iter && current_eps > eps_max) {
        current_eps = 0.0;

        // Обход снизу-вверх
        for (int j = 1; j < m; ++j) {
            double y = prob.c() + j * grid.k;
            
            // Обход слева-направо
            for (int i = 1; i < n; ++i) {
                double x = prob.a() + i * grid.h;

                double v_old = grid.data[i][j];

                double v_seidel = ( (grid.data[i-1][j] + grid.data[i+1][j]) * h2_inv + 
                                    (grid.data[i][j-1] + grid.data[i][j+1]) * k2_inv + 
                                    prob.f(x, y) ) / A;

                // Применяем верхнюю релаксацию
                double v_new = (1.0 - w) * v_old + w * v_seidel;

                // Считаем невязку
                double diff = std::abs(v_new - v_old);
                if (diff > current_eps) {
                    current_eps = diff;
                }

                // Обновляем значение
                grid.data[i][j] = v_new;
            }
        }
        iter++;
    }

    return {grid, iter, current_eps};
}