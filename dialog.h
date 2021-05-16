#pragma once

#include <QDialog>
#include <QDebug>
#include <QDomDocument>
#include "ConsoleSampleEventListener.h"
#include <map>
#include <set>
#include <thread>
#include <memory>

#include "pugixml.hpp"

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

ostream& operator<<( ostream&  out, CancelPos& pos ) {
    out << pos.reason_of_cancellation << " " << endl
        << pos.document_type << " " << endl
        << pos.document_number << " " << endl
        << pos.document_date << " " << endl
        << pos.document_name << " " << endl
        << pos.register_number_kkt << " " << endl
        << pos.price << " " << endl
        << pos.inn << endl
        << pos.expected << " " << endl
        << pos.current << " " << endl
        << pos.name  << " " << endl;

    return out;
}

struct Position
{
    std::string code_tn_ved;
    std::string document_type;
    std::string document_number;
    std::string document_date;

    std::string vsd;
    int expected;
    int current;

    std::string name;
    std::string inn;
};

class Pos {
    virtual std::string mode() = 0;
    virtual std::string to_string() = 0;
    virtual std::string name() = 0;
    virtual int& current() = 0;
    virtual void write_header(std::ofstream &) = 0;
    virtual std::string description() = 0;
    virtual std::vector<std::string> add_positions(const pugi::xml_document &) = 0;
};


ostream& operator<<( ostream&  out, Position& pos ) {
    out << pos.code_tn_ved << " " << endl
        << pos.document_type << " " << endl
        << pos.document_number << " " << endl
        << pos.document_date << " " << endl
        << pos.vsd << " " << endl
        << pos.expected << " " << endl
        << pos.current << " " << endl
        << pos.name  << " " << endl
        << pos.inn << endl;

    return out;
}

class InputPos : Pos {
    std::string mode() override {
        return "input";
    }

    virtual std::string to_string() override {
        std::stringstream ss;
        ss << currentPosition;
        return ss.str();
    }

    int& current() override {
        return currentPosition.current;
    }

    void write_header(std::ofstream & os) override {
        os << "ИНН участника оборота,ИНН производителя,ИНН собственника,Дата производства,Тип производственного заказа,Версия" << endl;
        os << currentPosition.inn << "," << currentPosition.inn << "," << currentPosition.inn << "," << date_buffer << ",Собственное производство,4" << endl;
        os << "КИ,КИТУ,Дата производства,Код ТН ВЭД ЕАС товара,Вид документа подтверждающего соответствие,Номер документа подтверждающего соответствие,Дата документа подтверждающего соответствие,Идентификатор ВСД" << endl;
    }

    std::vector<std::string> add_positions(const pugi::xml_document &doc) override {
        pugi::xml_node inn_xml = doc.child("resources").child("inn");
        pugi::xml_node version = doc.child("resources").child("version");

        pugi::xml_node positions_xml = doc.child("resources").child(mode().c_str());

        if (positions_xml.children("position").begin() == positions_xml.children("position").end()) {
            cout << "Fatal! No positions!" << endl;
            return {};
        }

        for (pugi::xml_node position_xml: positions_xml.children("position")) {
            std::string position_name = position_xml.attribute("name").as_string();

            Position position = Position{position_xml.attribute("code_tn_ved").as_string(),
                                         position_xml.attribute("document_type").as_string(),
                                         position_xml.attribute("document_number").as_string(),
                                         position_xml.attribute("document_date").as_string(),
                                         position_xml.attribute("vsd").as_string(),
                                         position_xml.attribute("expected").as_int(),
                                         position_xml.attribute("current").as_int(),
                                         position_name,
                                         inn_xml.text().get()};

            cout << "Считана позиция: " << endl << position << endl;
            positions.insert({position_name, position});
        }

        std::vector<std::string> names;

        for(auto const &[name, position]: positions)
            names.push_back(name);

        return names;
    }

    std::map<std::string, Position> positions;
    Position currentPosition;
};

class OutputPos : Pos {
    std::string mode() override {
        return "output";
    }

    virtual std::string to_string() override {
        std::stringstream ss;
        ss << cancelPos;
        return ss.str();
    }

    int& current() override {
        return cancelPos.current;
    }

    std::vector<std::string> add_positions(const pugi::xml_document &doc) override {
        pugi::xml_node inn_xml = doc.child("resources").child("inn");
        pugi::xml_node version = doc.child("resources").child("version");

        pugi::xml_node positions_xml = doc.child("resources").child(mode().c_str());

        if (positions_xml.children("position").begin() == positions_xml.children("position").end()) {
            cout << "Fatal! No positions!" << endl;
            return {};
        }

        for (pugi::xml_node position_xml: positions_xml.children("position")) {
            std::string position_name = position_xml.attribute("name").as_string();

            CancelPos cancelPos = CancelPos{position_xml.attribute("reason_of_cancellation").as_string(),
                                  position_xml.attribute("document_type").as_string(),
                                  position_xml.attribute("document_number").as_string(),
                                  position_xml.attribute("document_date").as_string(),
                                  position_xml.attribute("document_name").as_string(),
                                  position_xml.attribute("register_number_kkt").as_string(),
                                  position_xml.attribute("price").as_int(),
                                  inn_xml.text().get(),
                                  position_xml.attribute("expected").as_int(),
                                  position_xml.attribute("current").as_int(),
                                  position_name};

            cout << "Считана позиция: " << endl << cancelPos << endl;
            positions.insert({position_name, cancelPos});
        }

        std::vector<std::string> names;

        for(auto const &[name, position]: positions)
            names.push_back(name);

        return names;
    }

    std::map<std::string, CancelPos> positions;
    CancelPos cancelPos;
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




    SampleEventListener sel;
    const WF work_format;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_scan_time;
    std::thread comport_listener;
    bool stop_listen_comport = false;
    Timer timer;
    std::unique_ptr<Pos> pos_handler;
};
