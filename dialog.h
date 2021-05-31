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

#include "pugixml.hpp"

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
    int expected;
    int current;
    std::string name;
};

ostream& operator<<( ostream&  out, CancelPos& pos );

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

class IPos {
public:
    virtual std::string mode() = 0;
    virtual std::string to_string() = 0;
    virtual const std::string & name() = 0;
    virtual int& current() = 0;
    virtual int& expected() = 0;
    virtual void write_header(std::ofstream &) = 0;
    virtual void write_scan(const std::string &file_name, const std::string &bar_code) = 0;
    virtual void update_xml(const std::string &) = 0;
    virtual std::vector<std::string> names() = 0;
    virtual bool set_current_position_if_exists(const std::string &) = 0;
};


ostream& operator<<( ostream&  out, Position& pos );

class InputPos : public IPos {
public:
    InputPos(const std::string & config_path_)
        : config_path(config_path_) {
        add_positions();
    }

    std::string mode() override {
        return "input";
    }

    const std::string & name() override {
        return positions[current_name].name;
    }

    virtual std::string to_string() override {
        std::stringstream ss;
        ss << positions[current_name];
        return ss.str();
    }

    int& current() override {
        return positions[current_name].current;
    }

    int& expected() override {
        return positions[current_name].expected;
    }

    void write_header(std::ofstream & os) override {
        std::string date_pattern = std::string("%Y-%m-%d");
        char date_buffer [80];

        time_t t = time(0);
        struct tm * now = localtime( & t );
        strftime (date_buffer,80,date_pattern.c_str(),now);

        os << "ИНН участника оборота,ИНН производителя,ИНН собственника,Дата производства,Тип производственного заказа,Версия" << endl;
        os << positions[current_name].inn << "," << positions[current_name].inn << "," << positions[current_name].inn << "," << date_buffer << ",Собственное производство,4" << endl;
        os << "КИ,КИТУ,Дата производства,Код ТН ВЭД ЕАС товара,Вид документа подтверждающего соответствие,Номер документа подтверждающего соответствие,Дата документа подтверждающего соответствие,Идентификатор ВСД" << endl;
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
           << ","
           << date_buffer << ","
           << positions[current_name].code_tn_ved << ","
           << positions[current_name].document_type << ","
           << positions[current_name].document_number << ","
           << positions[current_name].document_date << ","
           << positions[current_name].vsd;
        std::cout << "positions[current_name].document_date: " << positions[current_name].document_date.size() << std::endl;

        std::cout << positions[current_name].vsd << std::endl;
        std::cout << positions[current_name].vsd.c_str() << std::endl;

        std::cout << positions[current_name].vsd.size() << std::endl;
        std::cout << strlen(positions[current_name].vsd.c_str()) << std::endl;

        ++positions[current_name].current;

        if(positions[current_name].current != positions[current_name].expected) {
            std::cout <<"current != expected" << std::endl;
            myfile << endl;
        } else {
            std::cout <<"current == expected" << std::endl;
        }

        myfile.close();

        update_xml("positions.xml");

        std::cout<<"Шаблон обновлен" << std::endl;
    }

    void update_xml(const std::string &xml_path) override {
        pugi::xml_document doc;

        if (not doc.load_file(xml_path.c_str())) {
            cout << "Не удалось загрузить XML документ" << endl;
            return;
        } else {
            cout << "Удалось загрузить XML документ" << endl;
        }

        pugi::xml_node positions_xml = doc.child("resources").child(mode().c_str());
        cout << "mode: " << mode().c_str() << endl;

        for (pugi::xml_node position_xml: positions_xml.children("position")) {
            std::string position_name = position_xml.attribute("name").as_string();
            if(position_name == current_name) {
                positions[current_name].current;
                cout << "current name: " << current_name << endl;
                cout << "current number: " << positions[current_name].current << endl;
                cout << "expected number: " << positions[current_name].expected << endl;
                position_xml.attribute("current").set_value(positions[current_name].current); // maybe set number??? TODO
                position_xml.attribute("expected").set_value(positions[current_name].expected);
            }
        }

        if(not doc.save_file(xml_path.c_str())) {
            cout << "Не удалось сохранить XML документ" << endl;
            return;
        }
    }

    void add_positions() {
        pugi::xml_document doc;
        if (!doc.load_file(config_path.c_str())) {
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
    }

    std::vector<std::string> names() override {
        std::vector<std::string> names;

        for(auto const &[name, position]: positions)
            names.push_back(name);

        return names;
    }

    bool set_current_position_if_exists(const std::string &name) override {
        auto it = positions.find(name);
        if(it != positions.end()) {
            current_name = name;
            cout << "Текущая позиция: " << endl << positions[current_name] << endl;
            return true;
        } else {
            cout << "Не найдено соответствие" << endl;
            return false;
        }
    }

private:
    std::map<std::string, Position> positions;
    std::string current_name;
    std::string config_path;
};

class OutputPos : public IPos {
public:
    OutputPos(const std::string & config_path_)
        : config_path(config_path_) {
        add_positions();
    }

    std::string mode() override {
        return "output";
    }

    virtual std::string to_string() override {
        std::stringstream ss;
        ss << positions[current_name];
        return ss.str();
    }

    const std::string & name() override {
        return positions[current_name].name;
    }

    int& current() override {
        return positions[current_name].current;
    }

    int& expected() override {
        return positions[current_name].expected;
    }

    void write_header(std::ofstream & os) override {
            std::string date_pattern = std::string("%Y-%m-%d");
            char date_buffer [80];

            time_t t = time(0);
            struct tm * now = localtime( & t );
            strftime (date_buffer,80,date_pattern.c_str(),now);

            os << "ИНН участника оборота,Причина вывода из оборота,Дата вывода из оборота,Тип первичного документа,Номер первичного документа,Дата первичного документа,Наименование первичного документа,Регистрационный номер ККТ,Версия" << endl;
            os << positions[current_name].inn << "," << positions[current_name].reason_of_cancellation << "," << date_buffer << "," << positions[current_name].document_type << "," << positions[current_name].document_number << "," << positions[current_name].document_date << "," << positions[current_name].document_name << "," << positions[current_name].register_number_kkt << ",4" << endl;
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
               << positions[current_name].price << ","
               << positions[current_name].document_type << ","
               << positions[current_name].document_number << ","
               << positions[current_name].document_date << ","
               << positions[current_name].document_name;

        ++positions[current_name].current;

        if(positions[current_name].current != positions[current_name].expected)
            myfile << endl;

        myfile.close();

        update_xml("positions.xml");

        std::cout<<"Шаблон обновлен" << std::endl;
    }

    void update_xml(const std::string &xml_path) override {
        pugi::xml_document doc;

        if (not doc.load_file(xml_path.c_str())) {
            cout << "Не удалось загрузить XML документ" << endl;
            return;
        } else {
            cout << "Удалось загрузить XML документ" << endl;
        }

        pugi::xml_node positions_xml = doc.child("resources").child(mode().c_str());
        cout << "mode: " << mode().c_str() << endl;

        for (pugi::xml_node position_xml: positions_xml.children("position")) {
            std::string position_name = position_xml.attribute("name").as_string();
            if(position_name == current_name) {
                positions[current_name].current;
                cout << "current name: " << current_name << endl;
                cout << "current number: " << positions[current_name].current << endl;
                cout << "expected number: " << positions[current_name].expected << endl;
                position_xml.attribute("current").set_value(positions[current_name].current);
                position_xml.attribute("expected").set_value(positions[current_name].expected);
            }
        }

        if(not doc.save_file(xml_path.c_str())) {
            cout << "Не удалось сохранить XML документ" << endl;
            return;
        }
    }

    void add_positions() {
        pugi::xml_document doc;
        if (!doc.load_file(config_path.c_str())) {
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
    }

    std::vector<std::string> names() override {
        std::vector<std::string> names;

        for(auto const &[name, position]: positions)
            names.push_back(name);

        return names;
    }

    bool set_current_position_if_exists(const std::string &name) override {
        auto it = positions.find(name);
        if(it != positions.end()) {
            current_name = name;
            cout << "Текущая позиция установлена: " << endl << positions[current_name] << endl;
            return true;
        } else {
            cout << "Не найдено соответствие названию" << name << endl;
            return false;
        }
    }

private:
    std::map<std::string, CancelPos> positions;
    std::string current_name;
    std::string config_path;
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
    explicit Dialog(WF work_format_, bool new_template, QWidget *parent = nullptr);
    ~Dialog();

    std::map<std::string, std::string> get_vsds(const std::string & vsd_path) {
        std::ifstream vsd;

        vsd.open(vsd_path);
        if(not vsd.is_open()) {
            std::cout<<"Ошибка открытия vsd.csv"<<std::endl;
            return {};
        } else {
            std::cout<<"Открытие vsd.csv успешно"<<std::endl;
        }

        std::map<std::string, std::string> vsds;
        string line;
        for (int i = 0; std::getline(vsd, line); ++i) {
            cout << "line: " << line << endl;
            cout << "line.size(): " << line.size() << endl;

            int pos = line.size() - 12;
            cout << "line.size() - 12: " << pos << endl;
            string name = line.substr(0, line.find(","));
            int comma_pos = line.find(",");
            int size_of_vsd = line.size() - 1 - (comma_pos + 1);
            cout << "comma_pos: " << comma_pos << endl;
            cout << "size_of_vsd: " << size_of_vsd << endl;
            string vsd = line.substr(comma_pos + 1, size_of_vsd);

            cout << "name: " << name << endl;
            cout << "vsd: " << vsd << endl;
            cout << "vsd.size(): " << vsd.size() << endl;

            vsds.emplace(name,vsd);
        }

        return vsds;
    }

    bool update_config_file (const std::string & path, const std::map<std::string, std::string> & vsd_per_names, bool new_template) {
        pugi::xml_document doc;
        if (!doc.load_file(path.c_str())) {
            cout << "Не удалось загрузить XML документ" << endl;
            return false;
        } else {
            cout << "Удалось загрузить XML документ" << endl;
        }

        for(auto &[name, vsd] : vsd_per_names) {
            pugi::xml_node positions_xml = doc.child("resources").child("input");

            for (pugi::xml_node position_xml: positions_xml.children("position")) {
                std::string position_name = position_xml.attribute("name").as_string();
                std::string cheese = string("Сыр мягкий \"Кавказский\"");
                if(name == "milk Byrenka" && position_name == "молоко Буренка") {
                    position_xml.attribute("vsd").set_value(vsd.c_str());
                    cout << "milk----" << endl;
                } else if(name == "sir Kavkazskiu" && position_name == cheese) {
                    position_xml.attribute("vsd").set_value(vsd.c_str());
                    cout << "cheese----" << endl;
                }
            }

            for (pugi::xml_node position_xml: positions_xml.children("position"))
                if(new_template)
                    position_xml.attribute("current").set_value(0);
        }

        if(not doc.save_file(path.c_str())) {
            cout << "Не удалось сохранить XML документ" << endl;
            return false;
        }

        return true;
    }

    void barCodeEvent(std::string bar_code);
    void write_scan(std::string bar_code);


    QString get_decode_data(std::string outXml);
    std::vector<std::string> stringTokernize(std::string inStr, char cDelim);
    bool codeExists(ifstream &myfile, const string &barCode);
    void listen_comport();
private slots:
    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_pushButton_clicked();
    void on_lineEdit_textChanged(const QString &arg1);
private:
    Ui::Dialog *ui;

    SampleEventListener sel;
    const WF work_format;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_scan_time;
    std::thread comport_listener;
    bool stop_listen_comport = false;
    Timer timer;
    std::unique_ptr<IPos> pos_handler;
};
