#pragma once

#include <QMainWindow>
#include "Qt/PoissonBackend.h"
#include "Problems/TestProblem.h"

class QSpinBox;
class QPushButton;
class QLabel;
class SurfaceWidget;

class PoissonWindow : public QMainWindow {

public:
    explicit PoissonWindow(QWidget* parent = nullptr);

    void onDrawClicked();

private:
    void drawSolution();

    Field2D exactField;
    Field2D numericField;
    TestProblem testProb;

    QSpinBox* xSpinBox;
    QSpinBox* ySpinBox;
    QPushButton* drawButton;
    QLabel* infoLabel;
    SurfaceWidget* surfaceWidget;

    int currentN;
    int currentM;
    double eps;
    int maxIter;
};
