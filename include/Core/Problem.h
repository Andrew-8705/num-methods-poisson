#pragma once

class Problem {
public:
    virtual ~Problem() = default;

    // Границы области
    virtual double a() const = 0;
    virtual double b() const = 0;
    virtual double c() const = 0;
    virtual double d() const = 0;

    // Правая часть уравнения Пуассона
    virtual double f(double x, double y) const = 0;

    // Граничные условия
    virtual double mu1(double y) const = 0; // Левая граница (x = a)
    virtual double mu2(double y) const = 0; // Правая граница (x = b)
    virtual double mu3(double x) const = 0; // Нижняя граница (y = c)
    virtual double mu4(double x) const = 0; // Верхняя граница (y = d)

    // Для тестовой задачи
    virtual bool has_exact_solution() const { return false; }
    virtual double exact_u(double x, double y) const { return 0.0; }
};