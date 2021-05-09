#pragma once

#include <QDialog>
#include <QDebug>
#include <QDomDocument>
#include "ConsoleSampleEventListener.h"
#include <map>


struct Position
{
    Position() {}
    Position(std::string code_tn_ved_,
             std::string document_type_,
             std::string document_number_,
             std::string document_date_,
             std::string vsd_,
             int expected_,
             int current_,
             std::string name_,
             std::string inn_):
        code_tn_ved(code_tn_ved_),
        document_type(document_type_),
        document_number(document_number_),
        document_date(document_date_),
        vsd(vsd_),
        expected(expected_),
        current(current_),
        name(name_),
        inn(inn_){
    }
    std::string code_tn_ved;
    std::string document_type;
    std::string document_number;
    std::string document_date;

    std::string vsd;
    int expected;
    int current;

    std::string name;
    std::string inn;

    Position& operator=(const Position &rhs)
    {
        code_tn_ved = rhs.code_tn_ved;
        document_type = rhs.document_type;
        document_number = rhs.document_number;
        document_date = rhs.document_date;

        vsd = rhs.vsd;
        expected = rhs.expected;
        current = rhs.current;

        name = rhs.name;
        inn = rhs.inn;

        return *this;
    }
};


namespace Ui {
class Dialog;
}

enum WF {
    vvod,
    vivod
};

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(WF work_format, QWidget *parent = nullptr);
    ~Dialog();

    void barCodeEvent(std::string barCode);

    QString get_decode_data(std::string outXml);
    std::vector<std::string> stringTokernize(std::string inStr, char cDelim);
private slots:
    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
    std::map<std::string, Position> positions;
    Position currentPosition;
    SampleEventListener sel;
};
