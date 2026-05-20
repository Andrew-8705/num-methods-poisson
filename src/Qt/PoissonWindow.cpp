#include "Qt/PoissonWindow.h"

#include <algorithm>
#include <cmath>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
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
    constexpr double cosA = 0.7660444431; // cos(40°)
    constexpr double sinA = 0.6427876097; // sin(40°)

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
        : QWidget(parent)
    {
        setMinimumSize(900, 600);
    }

    void setFields(const Field2D& exactField_, const Field2D& numericField_) {
        exact = exactField_;
        numeric = numericField_;
        update();
    }

protected:
    void paintEvent(QPaintEvent* /*event*/) override {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::white);

        QRect leftRect(10, 10, width() / 2 - 20, height() - 20);
        QRect rightRect(width() / 2 + 10, 10, width() / 2 - 20, height() - 20);

        drawSurface(painter, exact, leftRect, QStringLiteral("Точное решение"));
        drawSurface(painter, numeric, rightRect, QStringLiteral("Численное решение"));
    }

private:
    Field2D exact;
    Field2D numeric;

    void drawSurface(QPainter& painter, const Field2D& field, const QRect& area, const QString& title) {
        double minValue = field.values.empty() ? 0.0 : field.values[0];
        double maxValue = minValue;
        for (double value : field.values) {
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
        }

        struct Cell { double depth; int i; int j; };
        std::vector<Cell> cells;
        cells.reserve(field.n * field.m);

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
            quad << projectPoint(x0, y0, z00, field, area, minValue, maxValue);
            quad << projectPoint(x1, y0, z10, field, area, minValue, maxValue);
            quad << projectPoint(x1, y1, z11, field, area, minValue, maxValue);
            quad << projectPoint(x0, y1, z01, field, area, minValue, maxValue);

            QColor color(mapValueToColor(avgZ, minValue, maxValue));
            color.setAlpha(220);
            painter.setBrush(color);
            painter.drawPolygon(quad);
            painter.drawPolyline(quad);
        }

        QPointF origin = projectPoint(field.a, field.c, minValue, field, area, minValue, maxValue);
        QPointF xEnd = projectPoint(field.b, field.c, minValue, field, area, minValue, maxValue);
        QPointF yEnd = projectPoint(field.a, field.d, minValue, field, area, minValue, maxValue);
        QPointF zEnd = projectPoint(field.a, field.c, maxValue, field, area, minValue, maxValue);

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
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(area.adjusted(8, 8, -8, -8), Qt::AlignTop | Qt::AlignLeft, title);
        painter.setFont(QFont("Arial", 10));
        painter.drawText(area.adjusted(8, 28, -8, -8), Qt::AlignTop | Qt::AlignLeft,
                         QString("X: [%1, %2] Y: [%3, %4]").arg(field.a).arg(field.b).arg(field.c).arg(field.d));
    }
};

PoissonWindow::PoissonWindow(QWidget* parent)
    : QMainWindow(parent)
    , xSpinBox(nullptr)
    , ySpinBox(nullptr)
    , drawButton(nullptr)
    , infoLabel(nullptr)
    , surfaceWidget(nullptr)
    , currentN(50)
    , currentM(50)
    , eps(0.5e-6)
    , maxIter(10000)
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    QLabel* titleLabel = new QLabel(this);
    titleLabel->setText(QStringLiteral("Тестовая задача: численное решение и точное решение для СОР/Зейделя"));
    titleLabel->setAlignment(Qt::AlignHCenter);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

    auto* controlLayout = new QHBoxLayout();
    auto* formLayout = new QFormLayout();
    xSpinBox = new QSpinBox(this);
    xSpinBox->setRange(5, 200);
    xSpinBox->setValue(currentN);
    ySpinBox = new QSpinBox(this);
    ySpinBox->setRange(5, 200);
    ySpinBox->setValue(currentM);

    formLayout->addRow(QStringLiteral("Шагов по X:"), xSpinBox);
    formLayout->addRow(QStringLiteral("Шагов по Y:"), ySpinBox);

    drawButton = new QPushButton(QStringLiteral("Построить"), this);
    connect(drawButton, &QPushButton::clicked, this, &PoissonWindow::onDrawClicked);

    controlLayout->addLayout(formLayout);
    controlLayout->addWidget(drawButton);
    controlLayout->addStretch();

    infoLabel = new QLabel(this);
    infoLabel->setAlignment(Qt::AlignHCenter);
    infoLabel->setText(QStringLiteral("Итераций: -, Достигнутая точность: -"));

    surfaceWidget = new SurfaceWidget(this);

    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(infoLabel);
    mainLayout->addWidget(surfaceWidget, 1);

    setWindowTitle(QStringLiteral("Poisson Solver GUI"));
    resize(1000, 760);

    drawSolution();
}

void PoissonWindow::onDrawClicked() {
    currentN = xSpinBox->value();
    currentM = ySpinBox->value();
    drawSolution();
}

void PoissonWindow::drawSolution() {
    SolverResult result = PoissonBackend::solveTestProblem(currentN, currentM, eps, maxIter);
    exactField = PoissonBackend::exactSolution(testProb, currentN, currentM);
    numericField = PoissonBackend::fieldFromGrid(result.grid);
    surfaceWidget->setFields(exactField, numericField);
    infoLabel->setText(QStringLiteral("Итераций: %1, Достигнутая точность: %2")
            .arg(result.iterations)
            .arg(result.achieved_eps, 0, 'g', 6));
}
