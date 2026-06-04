#pragma once

#include <QMainWindow>
#include "Qt/PoissonBackend.h"
#include "Problems/TestProblem.h"

class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QLabel;
class QTabWidget;
class QTextEdit;
class QTableWidget;
class SurfaceWidget;
struct SolverResult;

class PoissonWindow : public QMainWindow {

public:
    explicit PoissonWindow(QWidget* parent = nullptr);

    void onDrawClicked();
    void onTabChanged(int index);

private:
    void drawSolution();
    void drawMainProblem();
    void updateReport(const SolverResult& result);
    void updateMainReport(const SolverResult& resultMain, const SolverResult& resultHalf,
                          double maxDiff, int maxI, int maxJ,
                          double xMax, double yMax,
                          double omegaMain, double omegaHalf);
    double computeOmega() const;
    QTableWidget* createTable1();
    QTableWidget* createMainTable();
    void fillTables();
    void fillMainTables();

    Field2D exactField;
    Field2D numericField;
    Field2D mainField;
    Field2D mainFieldHalf;
    TestProblem testProb;

    QSpinBox* xSpinBox;
    QSpinBox* ySpinBox;
    QDoubleSpinBox* omegaSpinBox;
    QDoubleSpinBox* epsSpinBox;
    QSpinBox* maxIterSpinBox;
    QCheckBox* autoOmegaCheck;
    QPushButton* drawButton;
    QTabWidget* taskTabs;
    SurfaceWidget* surfaceWidget;
    SurfaceWidget* mainSurfaceWidget;
    
    QTextEdit* helpTextEdit;    
    QTableWidget* resultTable;
    QTableWidget* mainResultTable;

    int currentN;
    int currentM;
    double currentOmega;
    double eps;
    int maxIter;
    int lastIterations;
    double lastAchievedEps;
    int mainLastIterations;
    double mainLastAchievedEps;
    int mainLastIterationsHalf;
    double mainLastAchievedEpsHalf;
};
