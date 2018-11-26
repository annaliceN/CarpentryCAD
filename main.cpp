#include "OCCWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    OCCWindow w;
    w.show();

    return a.exec();
}
