#include "quest.h"
#include "ui_quest.h"

quest::quest(WF work_format_, QWidget *parent) :
    QDialog(parent),
    work_format(work_format_),
    ui(new Ui::quest)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
}

quest::~quest()
{
    delete ui;
}

void quest::on_pushButton_2_clicked()
{
    bool new_template = false;
    f1 = new Dialog(work_format, new_template, this);
    hide();
    f1->exec();
}

void quest::on_pushButton_clicked()
{
    bool new_template = true;
    f1 = new Dialog(work_format, new_template, this);
    hide();
    f1->exec();
}
