#include "serverwin.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    serverWin w;
    w.show();

    return a.exec();
}
