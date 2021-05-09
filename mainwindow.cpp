#include "mainwindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    f1 = new Dialog(WF::vvod, this);
    hide();
    f1->exec();
}

void MainWindow::on_pushButton_2_clicked()
{
    f1 = new Dialog(WF::vivod, this);
    hide();
    f1->exec();
}
