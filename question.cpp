#include "question.h"
#include "ui_question.h"

question::question(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::question)
{
    ui->setupUi(this);
}

question::~question()
{
    delete ui;
}
