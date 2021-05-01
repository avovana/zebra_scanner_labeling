#pragma once

#include <QDialog>
#include <QDebug>
#include <QDomDocument>
#include "ConsoleSampleEventListener.h"
#include <map>


struct Position
{
    Position() {}
    Position(QString code_, std::string name_, std::string vsd_, int expected_, int current_):code(code_), name(name_), vsd(vsd_), expected(expected_), current(current_) {
    }
    QString code;
    std::string name;
    std::string vsd;
    int expected;
    int current;

    //friend QDataStream& operator>>( QDataStream& d, Position& u );
    Position& operator=(Position &rhs)
    {
        code = rhs.code;
        name = rhs.name;
        vsd = rhs.vsd;
        expected = rhs.expected;
        current = rhs.current;
        return *this;
    }
};


namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

    void barCodeEvent(std::string barCode);

    QString get_decode_data(std::string outXml);
    std::vector<std::string> stringTokernize(std::string inStr, char cDelim);
private slots:
    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
    std::map<QString, Position> positions;
    Position currentPosition;
    SampleEventListener sel;
};
