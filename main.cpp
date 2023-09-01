#include "mainwindow.h"
#include "sender.h"

#include <QApplication>

#include "ConsoleSampleEventListener.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    int res = a.exec();
    return res;
}
