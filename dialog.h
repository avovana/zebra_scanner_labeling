#pragma once

#include <QDialog>
#include <QDebug>
#include <QDomDocument>
#include "ConsoleSampleEventListener.h"
#include <map>
#include <set>
#include <thread>

class Timer
{
public:
    Timer() : beg_(clock_::now()) {}
    void reset() { beg_ = clock_::now(); }
    int elapsed() const {
        return std::chrono::duration_cast<ms>
            (clock_::now() - beg_).count(); }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::milliseconds ms;
    std::chrono::time_point<clock_> beg_;
};

struct CancelPos
{
    std::string reason_of_cancellation;
    std::string document_type;
    std::string document_number;
    std::string document_date;
    std::string document_name;
    std::string register_number_kkt;
    int price = 0;
    std::string inn;
    int expected;
    int current;
    std::string name;
};

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
    explicit Dialog(WF work_format_, QWidget *parent = nullptr);
    ~Dialog();

    void barCodeEvent(std::string barCode);

    QString get_decode_data(std::string outXml);
    std::vector<std::string> stringTokernize(std::string inStr, char cDelim);
    bool codeExists(ifstream &myfile, string &barCode);
    void listen_comport();
private slots:
    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
    std::set<std::string> codes;
    std::map<std::string, Position> positions;
    std::map<std::string, CancelPos> cancelPositions;
    Position currentPosition;
    CancelPos cancelPos;
    SampleEventListener sel;
    const WF work_format;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_scan_time;
    std::thread comport_listener;
    bool stop_listen_comport = false;
    Timer timer;
};
