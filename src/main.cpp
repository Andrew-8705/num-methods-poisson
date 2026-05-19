#include <iostream>
#include <cmath>
#include <iomanip>

#include "Problems/TestProblem.h"
#include "Problems/MainProblem.h"
#include "Solvers/SeidelSolver.h"
#include "Solvers/SORSolver.h"

// Вспомогательная функция для подсчета макс. ошибки тестовой задачи
double calculate_global_error(const Grid& grid, const Problem& prob) {
    double max_err = 0.0;
    for (int i = 0; i <= grid.n; ++i) {
        for (int j = 0; j <= grid.m; ++j) {
            double x = prob.a() + i * grid.h;
            double y = prob.c() + j * grid.k;
            double err = std::abs(grid.data[i][j] - prob.exact_u(x, y));
            if (err > max_err) max_err = err;
        }
    }
    return max_err;
}

int main() {
    // Параметры запуска
    int n = 30; // Число шагов по X
    int m = 30; // Число шагов по Y
    double eps = 0.5e-6; // Требуемая точность из методички
    int max_iter = 10000;

    std::cout << "===== ВАРИАНТ 1: ТЕСТОВАЯ ЗАДАЧА + МЕТОД ЗЕЙДЕЛЯ =====\n";
    TestProblem testProb;
    SeidelSolver seidel;
    
    SolverResult res1 = seidel.solve(testProb, n, m, eps, max_iter);
    
    std::cout << "Затрачено итераций N: " << res1.iterations << '\n';
    std::cout << "Достигнутая точность метода (невязка): " << res1.achieved_eps << '\n';
    std::cout << "Глобальная погрешность схемы e1: " << calculate_global_error(res1.grid, testProb) << "\n\n";

    
    std::cout << "===== ВАРИАНТ 4: ОСНОВНАЯ ЗАДАЧА + МЕТОД ВЕРХНЕЙ РЕЛАКСАЦИИ =====\n";
    MainProblem mainProb;
    
    // Для МВР нужно подобрать оптимальное Омега. 
    // Пока возьмем w = 1.5
    double omega = 1.5; 
    SORSolver sor(omega);
    
    SolverResult res4 = sor.solve(mainProb, n, m, eps, max_iter);
    
    std::cout << "Параметр МВР w: " << omega << '\n';
    std::cout << "Затрачено итераций N: " << res4.iterations << '\n';
    std::cout << "Достигнутая точность метода (невязка): " << res4.achieved_eps << '\n';

    return 0;
}