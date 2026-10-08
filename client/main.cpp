#include "clientwin.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    ClientWin w;
    w.show();

    return a.exec();
}
