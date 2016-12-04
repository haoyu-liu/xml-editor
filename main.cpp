#include <QApplication>
#include <QCoreApplication>
#include <QRect>
#include <QWidget>
#include <QDesktopWidget>

#include "textedit.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("XML editor");

    TextEdit te;

    const QRect geometry = QApplication::desktop()->availableGeometry(&te);
    te.resize(geometry.width()/2,geometry.height()/2);
    te.move(te.width()/2,te.height()/2);
    te.show();
    return a.exec();
}
