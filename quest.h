#ifndef QUEST_H
#define QUEST_H

#include <QDialog>

#include "dialog.h"

namespace Ui {
class quest;
}

class quest : public QDialog
{
    Q_OBJECT

public:
    explicit quest(WF work_format_, QWidget *parent = nullptr);
    ~quest();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    Ui::quest *ui;
    const WF work_format;
    Dialog *f1;
};

#endif // QUEST_H
