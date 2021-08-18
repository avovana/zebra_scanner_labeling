#include "mainwindow.h"
#include "sender.h"

#include <QApplication>

#include "ConsoleSampleEventListener.h"

int main(int argc, char *argv[])
{
    std::cout << "1" << std::endl;
    QApplication a(argc, argv);

    std::cout << "2" << std::endl;

    MainWindow w;
    std::cout << "7" << std::endl;
    w.show();

    int res = a.exec();
    return res;
}
