#include "mainwindow.h"
#include "./ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    f2 = new quest(WF::vvod,this);
    //f1 = new Dialog(WF::vvod, this);
    hide();
    f2->exec();
    //f1->exec();
}

void MainWindow::on_pushButton_2_clicked()
{
    f2 = new quest(WF::vivod,this);
    //f1 = new Dialog(WF::vivod, this);
    hide();
    f2->exec();
    //f1->exec();
}
