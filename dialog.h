#pragma once

#include <QDialog>
#include <QDebug>
#include <QDomDocument>
#include "ConsoleSampleEventListener.h"
#include <map>
#include <set>
#include <thread>
#include <memory>
#include <fstream>
#include <stdexcept>
#include <filesystem>

#include <sys/types.h>
#include <sys/stat.h>

#include "pugixml.hpp"

enum WF {
    vvod,
    vivod
};

class Timer
{
public:
    Timer() : time(clock::now()) {
    }
    void reset() {
        time = clock::now();
    }
    int elapsed() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - time).count();
    }

private:
    typedef std::chrono::high_resolution_clock clock;
    std::chrono::time_point<std::chrono::high_resolution_clock> time;
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
    std::string name;
    std::string name_english;
    int expected;
    int current;
};

ostream& operator<<( ostream&  out, CancelPos& pos );

struct Position
{
    std::string code_tn_ved;
    std::string document_type;
    std::string document_number;
    std::string document_date;

    std::string vsd;

    std::string name;
    std::string inn;
    std::string name_english;
    int expected;
    int current;
};

class IPos {
public:
    virtual std::string mode() = 0;
    virtual std::string to_string() = 0;
    virtual const std::string & name() = 0;
    virtual void write_header(std::ofstream &) = 0;
    virtual void write_scan(const std::string &file_name, const std::string &bar_code) = 0;
    virtual int expected() = 0;
    virtual int current() = 0;
    virtual void set_expected(int) = 0;
};


ostream& operator<<( ostream&  out, Position& pos );

class InputPos : public IPos {
public:
    InputPos(const std::string & name) {
        pugi::xml_document doc;
        if (!doc.load_file("positions.xml")) {
            cout << "Не удалось загрузить XML документ" << endl;
            return;
        } else {
            cout << "Удалось загрузить XML документ" << endl;
        }

        pugi::xml_node inn_xml = doc.child("resources").child("inn");
        pugi::xml_node version = doc.child("resources").child("version");

        pugi::xml_node positions_xml = doc.child("resources").child(mode().c_str());

        if (positions_xml.children("position").begin() == positions_xml.children("position").end()) {
            cout << "ОШИБКА! В документе нет позиций!" << endl;
            return;
        }

        for (pugi::xml_node position_xml: positions_xml.children("position")) {
            std::string position_name = position_xml.attribute("name_english").as_string();

            if(name == position_name) {
                position = Position{position_xml.attribute("code_tn_ved").as_string(),
                                             position_xml.attribute("document_type").as_string(),
                                             position_xml.attribute("document_number").as_string(),
                                             position_xml.attribute("document_date").as_string(),
                                             position_xml.attribute("vsd").as_string(),
                                             position_name,
                                             inn_xml.text().get(),
                                             position_xml.attribute("name_english").as_string(),
                                             position_xml.attribute("expected").as_int(),
                                             position_xml.attribute("current").as_int()};

                cout << "Считана позиция: " << endl << position << endl;
            }
        }
    }

    std::string mode() override {
        return "input";
    }

    const std::string & name() override {
        return position.name_english;
    }

    virtual std::string to_string() override {
        std::stringstream ss;
        ss << position;
        return ss.str();
    }

    void write_header(std::ofstream & os) override {
        std::string date_pattern = std::string("%Y-%m-%d");
        char date_buffer [80];

        time_t t = time(0);
        struct tm * now = localtime( & t );
        strftime (date_buffer,80,date_pattern.c_str(),now);

        os << "ИНН участника оборота,ИНН производителя,ИНН собственника,Дата производства,Тип производственного заказа,Версия" << endl;
        os << position.inn << "," << position.inn << "," << position.inn << "," << date_buffer << ",Собственное производство,4" << endl;
        os << "КИ,КИТУ,Дата производства,Код ТН ВЭД ЕАС товара,Вид документа подтверждающего соответствие,Номер документа подтверждающего соответствие,Дата документа подтверждающего соответствие,Идентификатор ВСД" << endl;
    }

    void write_scan(const std::string &file_name, const std::string &bar_code) override {
        cout << " =======================" << __PRETTY_FUNCTION__ << endl;

        std::cout<<"Шаблон обновлен" << std::endl;
        std::ofstream myfile;

        myfile.open (file_name, std::ios_base::app);
        if(not myfile.is_open()) {
            std::cout<<"Ошибка открытия шаблона для данной позиции" << std::endl;
            return;
        }

        std::string date_pattern = std::string("%Y-%m-%d");
        char date_buffer [80];

        time_t t = time(0);
        struct tm * now = localtime( & t );
        strftime (date_buffer,80,date_pattern.c_str(),now);

        myfile << bar_code << ","
           << ","
           << date_buffer << ","
           << position.code_tn_ved << ","
           << position.document_type << ","
           << position.document_number << ","
           << position.document_date << ","
           << position.vsd;
        std::cout << "position.document_date: " << position.document_date.size() << std::endl;

        std::cout << position.vsd << std::endl;
        std::cout << position.vsd.c_str() << std::endl;

        std::cout << position.vsd.size() << std::endl;
        std::cout << strlen(position.vsd.c_str()) << std::endl;

        std::cout<<"expected=" << position.expected << std::endl;
        std::cout<<"current=" << position.current << std::endl;
        ++position.current;

        if(position.current != position.expected) {
            myfile << endl;
            std::cout<< "write scan" << std::endl;
        }

        myfile.close();

        std::cout<<"Шаблон обновлен" << std::endl;
        cout << " =======================" << __PRETTY_FUNCTION__ << endl;
    }

    int expected() override {
        return position.expected;
    }

    int current() override {
        return position.current;
    }

    void set_expected(int expected) override {
        position.expected = expected;
    }

    Position position;
};

class OutputPos : public IPos {
public:
    OutputPos(const std::string & name) {
        pugi::xml_document doc;
        if (!doc.load_file("positions.xml")) {
            cout << "Не удалось загрузить XML документ" << endl;
            return;
        } else {
            cout << "Удалось загрузить XML документ" << endl;
        }

        pugi::xml_node inn_xml = doc.child("resources").child("inn");
        pugi::xml_node version = doc.child("resources").child("version");

        pugi::xml_node positions_xml = doc.child("resources").child(mode().c_str());

        if (positions_xml.children("position").begin() == positions_xml.children("position").end()) {
            cout << "ОШИБКА! В документе нет позиций!" << endl;
            return;
        }

        for (pugi::xml_node position_xml: positions_xml.children("position")) {
            std::string position_name = position_xml.attribute("name_english").as_string();

            if(name == position_name) {
                position = CancelPos{position_xml.attribute("reason_of_cancellation").as_string(),
                        position_xml.attribute("document_type").as_string(),
                        position_xml.attribute("document_number").as_string(),
                        position_xml.attribute("document_date").as_string(),
                        position_xml.attribute("document_name").as_string(),
                        position_xml.attribute("register_number_kkt").as_string(),
                        position_xml.attribute("price").as_int(),
                        inn_xml.text().get(),
                        position_name,
                        position_xml.attribute("name_english").as_string(),
                        position_xml.attribute("expected").as_int(),
                        position_xml.attribute("current").as_int()};

                cout << "Считана позиция: " << endl << position << endl;
            }
        }
    }

    std::string mode() override {
        return "output";
    }

    virtual std::string to_string() override {
        std::stringstream ss;
        ss << position;
        return ss.str();
    }

    const std::string & name() override {
        return position.name_english;
    }

    void write_header(std::ofstream & os) override {
            std::string date_pattern = std::string("%Y-%m-%d");
            char date_buffer [80];

            time_t t = time(0);
            struct tm * now = localtime( & t );
            strftime (date_buffer,80,date_pattern.c_str(),now);

            os << "ИНН участника оборота,Причина вывода из оборота,Дата вывода из оборота,Тип первичного документа,Номер первичного документа,Дата первичного документа,Наименование первичного документа,Регистрационный номер ККТ,Версия" << endl;
            os << position.inn << "," << position.reason_of_cancellation << "," << date_buffer << "," << position.document_type << "," << position.document_number << "," << position.document_date << "," << position.document_name << "," << position.register_number_kkt << ",4" << endl;
            os << "КИ,Цена за единицу,Тип первичного документа,Номер первичного документа,Дата первичного документа,Наименование первичного документа" << endl;

    }

    void write_scan(const std::string &file_name, const std::string &bar_code) override {
        std::ofstream myfile;

        myfile.open (file_name, std::ios_base::app);
        if(not myfile.is_open()) {
            std::cout<<"Ошибка открытия шаблона для данной позиции" << std::endl;
            return;
        }

        std::string date_pattern = std::string("%Y-%m-%d");
        char date_buffer [80];

        time_t t = time(0);
        struct tm * now = localtime( & t );
        strftime (date_buffer,80,date_pattern.c_str(),now);

        myfile << bar_code << ","
               << position.price << ","
               << position.document_type << ","
               << position.document_number << ","
               << position.document_date << ","
               << position.document_name;

        ++position.current;

        if(not position.current == position.expected)
            myfile << endl;

        myfile.close();

        std::cout<<"Шаблон обновлен" << std::endl;
    }

    int expected() override {
        return position.expected;
    }

    int current() override {
        return position.current;
    }

    void set_expected(int expected) override {
        position.expected = expected;
    }

    CancelPos position;
};

namespace Ui {
class Dialog;
}



class Dialog : public QDialog
{
    Q_OBJECT

public slots:
    void barCodeEvent(QString bar_code_);

public:
    explicit Dialog(std::unique_ptr<IPos> pos_handler_, QWidget *parent = nullptr);
    ~Dialog();


    QString get_decode_data(std::string outXml);
    std::vector<std::string> stringTokernize(std::string inStr, char cDelim);
    bool codeExists(string &maxPath, const string &barCode);
    void listen_comport();
    void update_expected(int);
private slots:
    void on_pushButton_clicked();
private:
    Ui::Dialog *ui;

    std::chrono::time_point<std::chrono::high_resolution_clock> last_scan_time;
    std::thread comport_listener;
    bool stop_listen_comport = false;
    Timer timer;
    std::unique_ptr<IPos> pos_handler;
    int km_number = 0;

    Sender sender;
    SampleEventListener sel;
};
