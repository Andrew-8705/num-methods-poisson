#pragma once

#include <QMainWindow>
#include "Qt/PoissonBackend.h"
#include "Problems/TestProblem.h"

class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QLabel;
class QTextEdit;
class QTableWidget;
class SurfaceWidget;
struct SolverResult;

class PoissonWindow : public QMainWindow {

public:
    explicit PoissonWindow(QWidget* parent = nullptr);

    void onDrawClicked();

private:
    void drawSolution();
    void updateReport(const SolverResult& result);
    double computeOmega() const;
    QTableWidget* createTable1();
    void fillTables();

    Field2D exactField;
    Field2D numericField;
    TestProblem testProb;

    QSpinBox* xSpinBox;
    QSpinBox* ySpinBox;
    QDoubleSpinBox* omegaSpinBox;
    QCheckBox* autoOmegaCheck;
    QPushButton* drawButton;
    QLabel* infoLabel;
    SurfaceWidget* surfaceWidget;
    
    QTextEdit* reportText;
    QTableWidget* resultTable;

    int currentN;
    int currentM;
    double currentOmega;
    double eps;
    int maxIter;
    int lastIterations;
    double lastAchievedEps;
};
