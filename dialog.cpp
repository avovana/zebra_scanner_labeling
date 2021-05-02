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

    cout << "Считаны позиции:" << endl;

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
    std::string date = std::string("%Y-%m-%d");

    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

    char date_buffer [80];
    strftime (date_buffer,80,date.c_str(),now);
    std::ofstream myfile;

    qDebug() << "current" << currentPosition.current;
    if(currentPosition.current == 0) {
        myfile.open (std::string(date_buffer) + " " + currentPosition.name);
        if(not myfile.is_open()) {
            std::cout<<"Error open file1"<<std::endl;
            return;
        }
        myfile << "ИНН участника оборота,ИНН производителя,ИНН собственника,Дата производства,Тип производственного заказа,Версия" << endl;
        myfile << currentPosition.inn << "," << currentPosition.inn << "," << currentPosition.inn << "," << date << ",Собственное производство,4" << endl;
        myfile << "КИ,КИТУ,Дата производства,Код ТН,ВЭД,ЕАС товара,Вид документа подтверждающего соответствие,Номер документа подтверждающего соответствие,Дата документа подтверждающего соответствие,Идентификатор ВСД" << endl;

        qDebug() << "Created file";
        myfile.close();
    }

    using std::cout; using std::cin;
    using std::endl; using std::string;
    using std::filesystem::directory_iterator;

    using std::filesystem::current_path;

    string path = current_path();
    cout << "path: " << path << endl;
    ulong max = 0;
    std::string maxPath = "";
    for (const auto & file : directory_iterator(path)) {
        std::string path_curr = file.path();
        //cout << path_curr << endl;

        std::string curName = currentPosition.name;

        std::size_t found = path_curr.find(curName);
          if (found == std::string::npos)
              continue;

        auto filename = file.path();
        struct stat result;
        if(stat(filename.c_str(), &result)==0)
        {
            auto mod_time = result.st_mtime;
            cout << path_curr << " " << mod_time << endl;

            if(mod_time > max) {
                max = mod_time;
                maxPath = path_curr;
            }
        }
    }

    cout << "maxPath: " << maxPath << " " << max << endl;

    myfile.open (maxPath, std::ios_base::app);
    if(not myfile.is_open()) {
        std::cout<<"Error open file2"<<std::endl;
        return;
    }

//    myfile << "0104630033880100211AREwAwLETM7g240ffd0,2019-01-01,6401921000,Сертификат соответствия,123,2019-01-01,9CDA-5D57-FAEA-46DD-B94D-3DCC-AC70-1091" << endl;
    myfile << barCode << "," << date_buffer <<",6401921000,Сертификат соответствия,123,2019-01-01," << currentPosition.vsd << endl;

    myfile.close();

    //==============================================

    QString filename = "positions.xml";
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Failed to open file";
        return;
    }

    QDomDocument document;
    if(!document.setContent(&file)){
        qDebug() << "Failed to load document";
        return;
    }

    file.close();

    QDomElement root = document.firstChildElement();
    qDebug() << "root.tagName(): " << root.tagName();

    QDomNodeList nodes = root.childNodes();
    for(int i = 0; i < nodes.count(); i++){
        QDomNode domNode = nodes.at(i);
        QDomElement domElement = domNode.toElement();
           qDebug() << "omElement.text() = " << domElement.text();
           qDebug() << "omElement.nodeName() = " << domElement.nodeName();
           qDebug() << "omElement.nodeValue() = " << domElement.nodeValue();
           if(domElement.text().toUtf8().constData() == currentPosition.name) {
               qDebug() << "1111" << domElement.text();
               ++currentPosition.current;
               domElement.setAttribute("current", currentPosition.current);
        }
    }

    //=========================

    if(file.open(QIODevice::WriteOnly)) {
            QTextStream(&file) << document.toString();
            file.close();
        }

    ui->label_7->setNum(currentPosition.current);

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
