#include <QApplication>
#include "Qt/PoissonWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    PoissonWindow window;
    window.show();
    return app.exec();
}