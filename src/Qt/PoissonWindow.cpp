#include "Qt/PoissonWindow.h"

#include <algorithm>
#include <cmath>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QWidget>
QRgb mapValueToColor(double value, double minValue, double maxValue) {
    if (maxValue <= minValue) {
        return qRgb(0, 120, 220);
    }
    double t = (value - minValue) / (maxValue - minValue);
    t = std::clamp(t, 0.0, 1.0);
    int g = static_cast<int>(40 + 160 * t);
    int b = static_cast<int>(140 + 115 * t);
    return qRgb(0, g, b);
}

QPointF projectPoint(double x, double y, double z, const Field2D& field, const QRect& area,
                     double minValue, double maxValue) {
    constexpr double cosA = 0.7660444431;
    constexpr double sinA = 0.6427876097;

    double nx = (x - field.a) / (field.b - field.a);
    double ny = (y - field.c) / (field.d - field.c);
    double nz = (z - minValue) / (maxValue - minValue);

    double px = (nx - ny) * cosA;
    double py = (nx + ny) * sinA - nz * 0.8;

    double scale = std::min(area.width(), area.height()) / 2.8;
    double cx = area.left() + area.width() * 0.5;
    double cy = area.top() + area.height() * 0.55;

    return QPointF(cx + px * scale, cy - py * scale);
}

class SurfaceWidget : public QWidget {
public:
    SurfaceWidget(QWidget* parent = nullptr)
        : QWidget(parent), rotationAngle(0.0), mousePressed(false), lastMouseX(0)
    {
        setMinimumSize(500, 400);
        setMaximumSize(550, 450);
        setAutoFillBackground(true);
        setStyleSheet("background-color: white;");
        setMouseTracking(false);
    }

    void setFields(const Field2D& leftField_, const Field2D& rightField_, const QString& leftTitle_, const QString& rightTitle_) {
        leftField = leftField_;
        rightField = rightField_;
        leftTitle = leftTitle_;
        rightTitle = rightTitle_;
        update();
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            mousePressed = true;
            lastMouseX = static_cast<int>(event->position().x());
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            mousePressed = false;
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (mousePressed) {
            int deltaX = static_cast<int>(event->position().x()) - lastMouseX;
            rotationAngle += deltaX * 0.5;
            lastMouseX = static_cast<int>(event->position().x());
            update();
        }
    }

    void paintEvent(QPaintEvent* /*event*/) override {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::white);

        QRect leftRect(10, 10, width() / 2 - 20, height() - 20);
        QRect rightRect(width() / 2 + 10, 10, width() / 2 - 20, height() - 20);

        painter.fillRect(leftRect, Qt::white);
        painter.fillRect(rightRect, Qt::white);

        drawSurface(painter, leftField, leftRect, leftTitle);
        drawSurface(painter, rightField, rightRect, rightTitle);
    }

private:
    Field2D leftField;
    Field2D rightField;
    QString leftTitle;
    QString rightTitle;
    double rotationAngle;
    bool mousePressed;
    int lastMouseX;

    QPointF projectPointWithRotation(double x, double y, double z, const Field2D& field, const QRect& area,
                                     double minValue, double maxValue) {
        // Применяем ротацию вокруг оси Z в 3D пространстве
        double radians = rotationAngle * M_PI / 180.0;
        
        // Сдвигаем в центр для ротации
        double cx_3d = (field.a + field.b) / 2.0;
        double cy_3d = (field.c + field.d) / 2.0;
        
        double x_rel = x - cx_3d;
        double y_rel = y - cy_3d;
        
        // Матрица ротации вокруг оси Z
        double x_rot = x_rel * std::cos(radians) - y_rel * std::sin(radians);
        double y_rot = x_rel * std::sin(radians) + y_rel * std::cos(radians);
        
        // Возвращаем в исходные координаты
        double x_final = x_rot + cx_3d;
        double y_final = y_rot + cy_3d;
        
        // Стандартная проекция с фиксированным углом
        constexpr double cosA = 0.7660444431;
        constexpr double sinA = 0.6427876097;

        double nx = (x_final - field.a) / (field.b - field.a);
        double ny = (y_final - field.c) / (field.d - field.c);
        double nz = (z - minValue) / (maxValue - minValue);

        double px = (nx - ny) * cosA;
        double py = (nx + ny) * sinA - nz * 0.8;

        double scale = std::min(area.width(), area.height()) / 2.8;
        double cx = area.left() + area.width() * 0.5;
        double cy = area.top() + area.height() * 0.55;

        return QPointF(cx + px * scale, cy - py * scale);
    }

    void drawSurface(QPainter& painter, const Field2D& field, const QRect& area, const QString& title) {
        // Проверка валидности поля
        if (field.n <= 0 || field.m <= 0 || field.values.empty()) {
            painter.fillRect(area, Qt::white);
            painter.drawRect(area);
            painter.drawText(area, Qt::AlignCenter, QStringLiteral("Нет данных"));
            return;
        }

        double minValue = field.values[0];
        double maxValue = minValue;
        for (double value : field.values) {
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
        }

        struct Cell { double depth; int i; int j; };
        std::vector<Cell> cells;
        cells.reserve(static_cast<size_t>(field.n) * static_cast<size_t>(field.m));

        for (int i = 0; i < field.n; ++i) {
            for (int j = 0; j < field.m; ++j) {
                double x = field.a + (i + 0.5) * (field.b - field.a) / field.n;
                double y = field.c + (j + 0.5) * (field.d - field.c) / field.m;
                double z = (field.at(i, j) + field.at(i + 1, j) + field.at(i, j + 1) + field.at(i + 1, j + 1)) * 0.25;
                cells.push_back({-(x + y) + z, i, j});
            }
        }

        std::sort(cells.begin(), cells.end(), [](const Cell& a, const Cell& b) {
            return a.depth < b.depth;
        });

        painter.fillRect(area, Qt::white);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor(64, 64, 64), 1));
        for (const Cell& cell : cells) {
            int i = cell.i;
            int j = cell.j;
            double x0 = field.a + i * (field.b - field.a) / field.n;
            double x1 = field.a + (i + 1) * (field.b - field.a) / field.n;
            double y0 = field.c + j * (field.d - field.c) / field.m;
            double y1 = field.c + (j + 1) * (field.d - field.c) / field.m;

            double z00 = field.at(i, j);
            double z10 = field.at(i + 1, j);
            double z01 = field.at(i, j + 1);
            double z11 = field.at(i + 1, j + 1);
            double avgZ = (z00 + z10 + z01 + z11) * 0.25;

            QPolygonF quad;
            quad << projectPointWithRotation(x0, y0, z00, field, area, minValue, maxValue);
            quad << projectPointWithRotation(x1, y0, z10, field, area, minValue, maxValue);
            quad << projectPointWithRotation(x1, y1, z11, field, area, minValue, maxValue);
            quad << projectPointWithRotation(x0, y1, z01, field, area, minValue, maxValue);

            QColor color(mapValueToColor(avgZ, minValue, maxValue));
            color.setAlpha(220);
            painter.setBrush(color);
            painter.drawPolygon(quad);
            painter.drawPolyline(quad);
        }

        QPointF origin = projectPointWithRotation(field.a, field.c, minValue, field, area, minValue, maxValue);
        QPointF xEnd = projectPointWithRotation(field.b, field.c, minValue, field, area, minValue, maxValue);
        QPointF yEnd = projectPointWithRotation(field.a, field.d, minValue, field, area, minValue, maxValue);
        QPointF zEnd = projectPointWithRotation(field.a, field.c, maxValue, field, area, minValue, maxValue);

        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(origin, xEnd);
        painter.drawLine(origin, yEnd);
        painter.drawLine(origin, zEnd);

        painter.setFont(QFont("Arial", 9, QFont::Bold));
        painter.drawText(xEnd + QPointF(4, 0), QStringLiteral("X"));
        painter.drawText(yEnd + QPointF(4, 0), QStringLiteral("Y"));
        painter.drawText(zEnd + QPointF(0, -6), QStringLiteral("Z"));

        painter.setPen(Qt::black);
        painter.drawRect(area);
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.drawText(area.adjusted(8, 8, -8, -8), Qt::AlignTop | Qt::AlignLeft, title);
        painter.setFont(QFont("Arial", 8));
        painter.drawText(area.adjusted(8, 24, -8, -8), Qt::AlignTop | Qt::AlignLeft,
                         QString("X: [%1, %2]\nY: [%3, %4]").arg(field.a).arg(field.b).arg(field.c).arg(field.d));
    }
};

PoissonWindow::PoissonWindow(QWidget* parent)
    : QMainWindow(parent)
    , xSpinBox(nullptr)
    , ySpinBox(nullptr)
    , drawButton(nullptr)
    , surfaceWidget(nullptr)
    , helpTextEdit(nullptr)      // ← добавлено
    , resultTable(nullptr)
    , mainResultTable(nullptr)
    , currentN(50)
    , currentM(50)
    , currentOmega(1.0)
    , eps(0.5e-6)
    , maxIter(10000)
    , lastIterations(0)
    , lastAchievedEps(0.0)
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    QLabel* titleLabel = new QLabel(this);
    titleLabel->setText(QStringLiteral("Численное решение уравнения Пуассона методом верхней релаксации"));
    titleLabel->setAlignment(Qt::AlignHCenter);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

    // ========== ПАНЕЛЬ УПРАВЛЕНИЯ СО СПРАВКОЙ СПРАВА ==========
    auto* controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(15);

    // Левая часть - поля ввода
    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(5);
    formLayout->setContentsMargins(0, 0, 0, 0);

    xSpinBox = new QSpinBox(this);
    xSpinBox->setRange(5, 10000);
    xSpinBox->setValue(currentN);
    ySpinBox = new QSpinBox(this);
    ySpinBox->setRange(5, 10000);
    ySpinBox->setValue(currentM);

    omegaSpinBox = new QDoubleSpinBox(this);
    omegaSpinBox->setRange(1.0, 2.0);
    omegaSpinBox->setSingleStep(0.01);
    omegaSpinBox->setDecimals(3);
    omegaSpinBox->setValue(1.0);

    epsSpinBox = new QDoubleSpinBox(this);
    epsSpinBox->setRange(1e-12, 1e-2);
    epsSpinBox->setSingleStep(1e-7);
    epsSpinBox->setDecimals(10);
    epsSpinBox->setValue(eps);

    maxIterSpinBox = new QSpinBox(this);
    maxIterSpinBox->setRange(10, 1000000);
    maxIterSpinBox->setSingleStep(10);
    maxIterSpinBox->setValue(maxIter);

    autoOmegaCheck = new QCheckBox(QStringLiteral("Автоподбор ω"), this);
    autoOmegaCheck->setChecked(true);
    omegaSpinBox->setEnabled(false);
    connect(autoOmegaCheck, &QCheckBox::toggled, this, [this](bool checked) {
        omegaSpinBox->setEnabled(!checked);
        });

    formLayout->addRow(QStringLiteral("n (шагов X):"), xSpinBox);
    formLayout->addRow(QStringLiteral("m (шагов Y):"), ySpinBox);
    formLayout->addRow(QStringLiteral("ω:"), omegaSpinBox);
    formLayout->addRow(QStringLiteral("εмет:"), epsSpinBox);
    formLayout->addRow(QStringLiteral("Nmax:"), maxIterSpinBox);
    formLayout->addRow(autoOmegaCheck);

    drawButton = new QPushButton(QStringLiteral("Построить"), this);
    connect(drawButton, &QPushButton::clicked, this, &PoissonWindow::onDrawClicked);

    controlLayout->addLayout(formLayout);
    controlLayout->addWidget(drawButton);
    controlLayout->addSpacing(20);

    // ========== СПРАВКА СПРАВА (с детальным отчетом) ==========
    auto* helpContainer = new QWidget(this);
    helpContainer->setMinimumWidth(450);
    helpContainer->setMaximumWidth(55000);
    helpContainer->setStyleSheet("background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 6px;");

    auto* helpLayout = new QVBoxLayout(helpContainer);
    helpLayout->setContentsMargins(10, 8, 10, 8);
    helpLayout->setSpacing(5);

    QLabel* helpTitle = new QLabel(QStringLiteral("Справка"), this);
    helpTitle->setStyleSheet("font-weight: bold; font-size: 12px; color: #2c3e50;");

    // Текстовое поле для детального отчета
    helpTextEdit = new QTextEdit(this);
    helpTextEdit->setReadOnly(true);
    helpTextEdit->setMinimumHeight(120);
    //helpTextEdit->setMaximumHeight(150);
    helpTextEdit->setFontPointSize(8);
    helpTextEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: white;"
        "    border: 1px solid #ddd;"
        "    border-radius: 3px;"
        "    padding: 5px;"
        "}"
    );

    helpLayout->addWidget(helpTitle);
    helpLayout->addWidget(helpTextEdit);

    controlLayout->addWidget(helpContainer, 1);
    //controlLayout->addStretch();

    // ========== ВКЛАДКИ С ЗАДАЧАМИ (без отчетов) ==========
    taskTabs = new QTabWidget(this);

    // ----- Тестовая задача (без детального отчета) -----
    QWidget* testPage = new QWidget(this);
    auto* testPageLayout = new QHBoxLayout(testPage);
    auto* testLeftPanel = new QVBoxLayout();
    surfaceWidget = new SurfaceWidget(this);
    testLeftPanel->addWidget(surfaceWidget);
    testLeftPanel->addStretch();  // график растягивается на всю высоту

    auto* testRightPanel = new QVBoxLayout();
    testRightPanel->setSpacing(8);
    testRightPanel->setContentsMargins(8, 0, 8, 0);

    resultTable = createTable1();
    resultTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    testRightPanel->addWidget(resultTable, 1);

    testPageLayout->addLayout(testLeftPanel, 1);
    testPageLayout->addLayout(testRightPanel, 1);

    // ----- Основная задача (без детального отчета) -----
    QWidget* mainPage = new QWidget(this);
    auto* mainPageLayout = new QHBoxLayout(mainPage);
    auto* mainLeftPanel = new QVBoxLayout();
    mainSurfaceWidget = new SurfaceWidget(this);
    mainLeftPanel->addWidget(mainSurfaceWidget);
    mainLeftPanel->addStretch();

    auto* mainRightPanel = new QVBoxLayout();
    mainRightPanel->setSpacing(8);
    mainRightPanel->setContentsMargins(8, 0, 8, 0);

    mainResultTable = createMainTable();
    mainResultTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainResultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mainRightPanel->addWidget(mainResultTable, 1);

    mainPageLayout->addLayout(mainLeftPanel, 1);
    mainPageLayout->addLayout(mainRightPanel, 1);

    taskTabs->addTab(testPage, QStringLiteral("Тестовая задача"));
    taskTabs->addTab(mainPage, QStringLiteral("Основная задача"));
    connect(taskTabs, &QTabWidget::currentChanged, this, &PoissonWindow::onTabChanged);

    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(taskTabs, 1);

    setWindowTitle(QStringLiteral("Poisson Solver GUI"));
    resize(1400, 900);

    drawSolution();
}
QTableWidget* PoissonWindow::createTable1() {
    QTableWidget* table = new QTableWidget(this);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("i, j"),
                                      QStringLiteral("x, y"),
                                      QStringLiteral("u*(x,y)"),
                                      QStringLiteral("u(N)(x,y)"),
                                      QStringLiteral("|Δu|")});
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return table;
}

QTableWidget* PoissonWindow::createMainTable() {
    QTableWidget* table = new QTableWidget(this);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("i, j"),
                                      QStringLiteral("x, y"),
                                      QStringLiteral("u_N(x,y)"),
                                      QStringLiteral("u_{2N}(x,y)"),
                                      QStringLiteral("|Δu|")});
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return table;
}

void PoissonWindow::onDrawClicked() {
    currentN = xSpinBox->value();
    currentM = ySpinBox->value();
    eps = epsSpinBox->value();
    maxIter = maxIterSpinBox->value();
    if (taskTabs->currentIndex() == 0) {
        drawSolution();
    } else {
        drawMainProblem();
    }
}

double PoissonWindow::computeOmega() const {
    if (autoOmegaCheck && autoOmegaCheck->isChecked()) {
        return PoissonBackend::calculateOptimalOmega(currentN, currentM,
                testProb.a(), testProb.b(), testProb.c(), testProb.d());
    }
    return omegaSpinBox ? omegaSpinBox->value() : 1.0;
}

void PoissonWindow::drawSolution() {
    currentOmega = computeOmega();
    SolverResult result = PoissonBackend::solveTestProblem(currentN, currentM, eps, maxIter, currentOmega);
    exactField = PoissonBackend::exactSolution(testProb, currentN, currentM);
    numericField = PoissonBackend::fieldFromGrid(result.grid);
    surfaceWidget->setFields(exactField, numericField,
                             QStringLiteral("Точное решение"), QStringLiteral("Численное решение"));
    
    lastIterations = result.iterations;
    lastAchievedEps = result.achieved_eps;
    
    updateReport(result);
    fillTables();
}

void PoissonWindow::updateReport(const SolverResult& result) {
    double globalError = PoissonBackend::calculateGlobalError(exactField, numericField);
    double maxDiff = 0.0;
    int maxI = 0;
    int maxJ = 0;
    for (int i = 0; i <= exactField.n; ++i) {
        for (int j = 0; j <= exactField.m; ++j) {
            double diff = std::abs(exactField.at(i, j) - numericField.at(i, j));
            if (diff > maxDiff) {
                maxDiff = diff;
                maxI = i;
                maxJ = j;
            }
        }
    }

    double xMax = exactField.a + maxI * (exactField.b - exactField.a) / exactField.n;
    double yMax = exactField.c + maxJ * (exactField.d - exactField.c) / exactField.m;

    QString report = QStringLiteral(
        "Для решения тестовой задачи использованы сетка с числом разбиений по x "
        "n=%1 и числом разбиений по y m=%2,\n"
        "метод верхней релаксации с параметром ω=%9, применены критерии "
        "остановки по точности ε_мет=%3 и по числу итераций N_max=%4.\n"
        "\n"
        "На решение схемы (СЛАУ) затрачено итераций N=%5 "
        "и достигнута точность итерационного метода ε(N)=%6.\n"
        "Схема (СЛАУ) решена с невязкой || R(N)|| = %8, использована норма max\n"
        "\n"
        "Тестовая задача должна быть решена с погрешностью не более ε = 0.5⋅10^-6;\n"
        "задача решена с погрешностью ε1 = %7\n"
        "\n"
        "Максимальное отклонение точного и численного решений наблюдается в узле x=%10; y=%11\n"
        "\n"
        "В качестве начального приближения использована интерполяция по x."
    ).arg(currentN).arg(currentM)
     .arg(eps, 0, 'e', 2)
     .arg(maxIter)
     .arg(result.iterations)
     .arg(result.achieved_eps, 0, 'e', 2)
     .arg(globalError, 0, 'e', 2)
     .arg(result.max_residual, 0, 'e', 2)
     .arg(currentOmega, 0, 'f', 4)
     .arg(xMax, 0, 'g', 5)
     .arg(yMax, 0, 'g', 5);
    
    if (helpTextEdit) {
        helpTextEdit->setText(report);
    }
}

void PoissonWindow::onTabChanged(int /*index*/) {
    // Построение происходит только по кнопке "Построить".
}

void PoissonWindow::drawMainProblem() {
    double omegaMain = computeOmega();
    double omegaHalf;
    if (autoOmegaCheck && autoOmegaCheck->isChecked()) {
        omegaHalf = PoissonBackend::calculateOptimalOmega(currentN * 2, currentM * 2,
                                                           testProb.a(), testProb.b(), testProb.c(), testProb.d());
    } else {
        omegaHalf = omegaSpinBox ? omegaSpinBox->value() : 1.0;
    }

    SolverResult resultMain = PoissonBackend::solveMainProblem(currentN, currentM, eps, maxIter, omegaMain);
    SolverResult resultHalf = PoissonBackend::solveMainProblem(currentN * 2, currentM * 2, eps, maxIter, omegaHalf);

    mainField = PoissonBackend::fieldFromGrid(resultMain.grid);
    mainFieldHalf = PoissonBackend::fieldFromGrid(resultHalf.grid);
    
    if (mainField.n <= 0 || mainField.m <= 0 || mainFieldHalf.n <= 0 || mainFieldHalf.m <= 0) {
        helpTextEdit->setText(QStringLiteral("Ошибка при решении задачи"));
        return;
    }

    mainSurfaceWidget->setFields(mainField, mainFieldHalf,
                                 QStringLiteral("Численное решение"), QStringLiteral("Численное решение (половина шага)"));

    mainLastIterations = resultMain.iterations;
    mainLastAchievedEps = resultMain.achieved_eps;
    mainLastIterationsHalf = resultHalf.iterations;
    mainLastAchievedEpsHalf = resultHalf.achieved_eps;

    double maxDiff = 0.0;
    int maxI = 0, maxJ = 0;
    if (mainFieldHalf.n == mainField.n * 2 && mainFieldHalf.m == mainField.m * 2) {
        for (int i = 0; i <= mainField.n; ++i) {
            for (int j = 0; j <= mainField.m; ++j) {
                double coarse = mainField.at(i, j);
                double halfStep = mainFieldHalf.at(2 * i, 2 * j);
                double diff = std::abs(coarse - halfStep);
                if (diff > maxDiff) {
                    maxDiff = diff;
                    maxI = i;
                    maxJ = j;
                }
            }
        }
    }

    double xMax = mainField.a + maxI * (mainField.b - mainField.a) / mainField.n;
    double yMax = mainField.c + maxJ * (mainField.d - mainField.c) / mainField.m;

    updateMainReport(resultMain, resultHalf, maxDiff, maxI, maxJ, xMax, yMax, omegaMain, omegaHalf);
    fillMainTables();
}

void PoissonWindow::updateMainReport(const SolverResult& resultMain, const SolverResult& resultHalf,
                                     double maxDiff, int maxI, int maxJ,
                                     double xMax, double yMax, double omegaMain, double omegaHalf) {
    QString report = QStringLiteral(
        "Для решения основной задачи использована сетка с числом разбиений по x "
        "n = %1 и числом разбиений по y m = %2,\n"
        "метод верхней релаксации с параметром ω = %7, применены критерии остановки по точности "
        "εмет = %3 "
        "и по числу итераций Nmax = %4\n"
        "\n"
        "На решение схемы (СЛАУ) затрачено итераций N = %5 "
        "и достигнута точность итерационного метода ε(N) = %6\n"
        "\n"
        "Схема (СЛАУ) решена с невязкой || R(N)|| = %8, использована норма max\n"
        "\n"
        "Для контроля точности решения использована сетка с половинным шагом, "
        "метод верхней релаксация с параметром ω2 = %15,\n"
        "применены критерии остановки по точности εмет-2 = %3 и по числу итераций Nmax-2 = %4\n"
        "\n"
        "На решение задачи (СЛАУ) затрачено итераций N2 = %9 и достигнута точность итерационного метода ε(N2) = %10\n"
        "\n"
        "Схема (СЛАУ) на сетке с половинным шагом решена с невязкой || R(N2)|| = %11, использована норма max\n"
        "\n"
        "Основная задача должна быть решена с точностью не хуже чем ε = 0.5⋅10^-6;\n"
        "задача решена с точностью ε2 = %14\n"
        "\n"
        "Максимальное отклонение численных решений на основной сетке и сетке с "
        "половинным шагом наблюдается в узле x=%12; y=%13\n"
        "В качестве начального приближения использована интерполяция по x."
    ).arg(currentN).arg(currentM)
     .arg(eps, 0, 'e', 2)
     .arg(maxIter)
     .arg(resultMain.iterations)
     .arg(resultMain.achieved_eps, 0, 'e', 2)
     .arg(omegaMain, 0, 'f', 4)
     .arg(resultMain.max_residual, 0, 'e', 2)
     .arg(resultHalf.iterations)
     .arg(resultHalf.achieved_eps, 0, 'e', 2)
     .arg(resultHalf.max_residual, 0, 'e', 2)
     .arg(xMax, 0, 'g', 5)
     .arg(yMax, 0, 'g', 5)
     .arg(maxDiff, 0, 'e', 2)
     .arg(omegaHalf, 0, 'f', 4);

    if (helpTextEdit) {
        helpTextEdit->setText(report);
    }
}

void PoissonWindow::fillMainTables() {
    if (!mainResultTable) {
        return;
    }

    mainResultTable->setRowCount(0);
    mainResultTable->setUpdatesEnabled(false);

    int row = 0;
    for (int i = 0; i <= mainField.n; ++i) {
        for (int j = 0; j <= mainField.m; ++j) {
            mainResultTable->insertRow(row);
            double x = mainField.a + i * (mainField.b - mainField.a) / mainField.n;
            double y = mainField.c + j * (mainField.d - mainField.c) / mainField.m;
            double coarse = mainField.at(i, j);
            double halfStep = 0.0;
            if (mainFieldHalf.n == mainField.n * 2 && mainFieldHalf.m == mainField.m * 2) {
                halfStep = mainFieldHalf.at(2 * i, 2 * j);
            }
            double diff = std::abs(coarse - halfStep);

            mainResultTable->setItem(row, 0, new QTableWidgetItem(QStringLiteral("%1, %2").arg(i).arg(j)));
            mainResultTable->setItem(row, 1, new QTableWidgetItem(QStringLiteral("%1, %2").arg(x, 0, 'g', 5).arg(y, 0, 'g', 5)));
            mainResultTable->setItem(row, 2, new QTableWidgetItem(QString::number(coarse, 'g', 5)));
            mainResultTable->setItem(row, 3, new QTableWidgetItem(QString::number(halfStep, 'g', 5)));
            mainResultTable->setItem(row, 4, new QTableWidgetItem(QString::number(diff, 'e', 2)));
            row++;
        }
    }

    mainResultTable->setUpdatesEnabled(true);
}

void PoissonWindow::fillTables() {
    if (!resultTable) {
        return;
    }

    resultTable->setRowCount(0);
    
    // Блокируем обновление UI на время заполнения, чтобы не было фризов
    resultTable->setUpdatesEnabled(false); 

    int row = 0;
    for (int i = 0; i <= currentN; ++i) { // Идем строго по всем i
        for (int j = 0; j <= currentM; ++j) { // Идем строго по всем j
            resultTable->insertRow(row);
            double x = exactField.a + i * (exactField.b - exactField.a) / exactField.n;
            double y = exactField.c + j * (exactField.d - exactField.c) / exactField.m;
            double exact = exactField.at(i, j);
            double numeric = numericField.at(i, j);
            double diff = std::abs(exact - numeric);

            resultTable->setItem(row, 0, new QTableWidgetItem(QStringLiteral("%1, %2").arg(i).arg(j)));
            resultTable->setItem(row, 1, new QTableWidgetItem(QStringLiteral("%1, %2").arg(x, 0, 'g', 5).arg(y, 0, 'g', 5)));
            resultTable->setItem(row, 2, new QTableWidgetItem(QString::number(exact, 'g', 5)));
            resultTable->setItem(row, 3, new QTableWidgetItem(QString::number(numeric, 'g', 5)));
            resultTable->setItem(row, 4, new QTableWidgetItem(QString::number(diff, 'e', 2)));
            row++;
        }
    }

    // Включаем отрисовку обратно
    resultTable->setUpdatesEnabled(true); 
}
