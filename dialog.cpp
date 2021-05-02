#include "dialog.h"
#include "ui_dialog.h"

#include <QMainWindow>
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QDomDocument>
#include <fstream>

#include <filesystem>

#include <sys/types.h>
#include <sys/stat.h>

#include "pugixml.hpp"

namespace fs = std::filesystem;
using namespace std::chrono_literals;

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

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    sel(*this)
{
    ui->setupUi(this);

    pugi::xml_document doc;
    if (!doc.load_file("positions.xml")) {
        cout << "Не удалось загрузить XML документ" << endl;
        return;
    }

    pugi::xml_node inn_xml = doc.child("resources").child("inn");
    pugi::xml_node positions_xml = doc.child("resources").child("positions");

    for (pugi::xml_node position_xml: positions_xml.children("position")) {
        std::string position_name = position_xml.attribute("name").as_string();

        Position position = Position(position_xml.attribute("code_tn_ved").as_string(),
                                     position_xml.attribute("document_type").as_string(),
                                     position_xml.attribute("document_number").as_string(),
                                     position_xml.attribute("document_date").as_string(),
                                     position_xml.attribute("vsd").as_string(),
                                     position_xml.attribute("expected").as_int(),
                                     position_xml.attribute("current").as_int(),
                                     position_name,
                                     inn_xml.text().get());

        cout << "Считана позиция: " << endl << position << endl;
        positions.insert({position_name, position});
        ui->comboBox->addItem(QString::fromStdString(position_name));
    }

    if(positions.empty())
        cout << "Ошибка! Позиции не считаны!" << endl;
}

Dialog::~Dialog()
{
    sel.Close();
    delete ui;
}

void Dialog::barCodeEvent(string barCode)
{
    std::string date_pattern = std::string("%Y-%m-%d");
    std::string filename_pattern = std::string("%Y-%m-%d:%H-%M");

    time_t t = time(0);
    struct tm * now = localtime( & t );

    char date_buffer [80];
    char filename_buffer [80];
    strftime (date_buffer,80,date_pattern.c_str(),now);
    strftime (filename_buffer,80,filename_pattern.c_str(),now);
    std::ofstream myfile;

    cout << endl << "Сохранение скана для текущей позиции: " << endl << currentPosition << endl;

    string filename = std::string(filename_buffer) + " " + currentPosition.name;
    if(currentPosition.current == 0) {
        myfile.open(filename);
        if(not myfile.is_open()) {
            std::cout<<"Ошибка создания файла шаблона"<<std::endl;
            return;
        }
        myfile << "ИНН участника оборота,ИНН производителя,ИНН собственника,Дата производства,Тип производственного заказа,Версия" << endl;
        myfile << currentPosition.inn << "," << currentPosition.inn << "," << currentPosition.inn << "," << date_buffer << ",Собственное производство,4" << endl;
        myfile << "КИ,КИТУ,Дата производства,Код ТН,ВЭД,ЕАС товара,Вид документа подтверждающего соответствие,Номер документа подтверждающего соответствие,Дата документа подтверждающего соответствие,Идентификатор ВСД" << endl;

        cout <<  "Создан новый шаблон для этой позиции: " << filename << endl;
        myfile.close();
    }

    using std::cout; using std::cin;
    using std::endl; using std::string;
    using std::filesystem::directory_iterator;

    using std::filesystem::current_path;

    string path = current_path();

    ulong max = 0;
    std::string maxPath = "";

    cout << "Шаблоны этой позиции:" << endl;
    for (const auto & file : directory_iterator(path)) {
        std::string path_curr = file.path();

        std::string curName = currentPosition.name;

        std::size_t found = path_curr.find(curName);
        if (found == std::string::npos)
            continue;

        auto filename = file.path();
        struct stat result;
        if(stat(filename.c_str(), &result)==0)
        {
            auto mod_time = result.st_mtime;
            cout << path_curr << endl;

            if(mod_time > max) {
                max = mod_time;
                maxPath = path_curr;
            }
        }
    }

    cout << "Выбран самый последний из шаблонов для данной позиции: " << maxPath << endl;

    myfile.open (maxPath, std::ios_base::app);
    if(not myfile.is_open()) {
        std::cout<<"Ошибка открытия шаблона для данной позиции" << std::endl;
        return;
    }

    myfile << barCode << ","
           << ","
           << date_buffer << ","
           << currentPosition.code_tn_ved << ","
           << currentPosition.document_type << ","
           << currentPosition.document_number << ","
           << currentPosition.document_date << ","
           << currentPosition.vsd << endl;

    myfile.close();

    std::cout<<"Шаблон сохранен" << std::endl;

    pugi::xml_document doc;
    if (not doc.load_file("positions.xml")) {
        cout << "Не удалось загрузить XML документ" << endl;
        return;
    }

    pugi::xml_node positions_xml = doc.child("resources").child("positions");

    for (pugi::xml_node position_xml: positions_xml.children("position")) {
        std::string position_name = position_xml.attribute("name").as_string();
        if(position_name == currentPosition.name) {
            ++currentPosition.current;
            positions[currentPosition.name].current++;
            position_xml.attribute("current").set_value(currentPosition.current);
        }
    }

    if(not doc.save_file("positions.xml")) {
        cout << "Не удалось сохранить XML документ" << endl;
        return;
    }

    ui->label_7->setNum(currentPosition.current);

    cout << "Сохранение скана для текущей позиции завершено успешно" << endl;
}

void Dialog::on_comboBox_currentTextChanged(const QString &arg1)
{
    auto it = positions.find(arg1.toUtf8().constData());
    if(it != positions.end()) {
        currentPosition = it->second;
        ui->label_6->setNum(currentPosition.expected);
        ui->label_7->setNum(currentPosition.current);
        cout << "Текущая позиция: " << endl << currentPosition << endl;
    } else {
        cout << "Не найдено соответствие" << endl;
    }
}

void Dialog::on_pushButton_clicked()
{
    if(STATUS_OK != sel.Open())
        qDebug() << "Corescanner service is not Active.";

    sel.GetScanners();
}
