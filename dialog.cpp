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

QDebug operator<<( QDebug d, Position& u ) {
    d << u.code << " " << QString::fromUtf8(u.name.c_str())  << " " <<u.expected << " " <<u.current << " " << QString::fromUtf8(u.vsd.c_str());
    return d;
}

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    sel(*this)
{
    ui->setupUi(this);

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

    Position position(domElement.attribute("code", ""), domElement.text().toUtf8().constData(), domElement.attribute("vsd", "").toUtf8().constData(), domElement.attribute("expected", "").toInt(), domElement.attribute("current", "").toInt());
    qDebug() << position;
    positions.insert({domElement.text(),position});
    ui->comboBox->addItem(domElement.text());
}

}

Dialog::~Dialog()
{
    sel.Close();
    delete ui;
}

void Dialog::barCodeEvent(string barCode)
{
    //Запись в файл.
    //std::ofstream outfile;

    std::string fileName = std::string("%Y.%m.%d-%H:%M");
    //outfile.open(fileName, std::ios_base::app); // append instead of overwrite

    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

    char buffer [80];
    strftime (buffer,80,fileName.c_str(),now);
    std::ofstream myfile;

    qDebug() << "current" << currentPosition.current;
    if(currentPosition.current == 0) {
        qDebug() << "sdfsdfds";
        myfile.open (std::string(buffer) + " " + currentPosition.name);
        if(not myfile.is_open()) {
            std::cout<<"Error open file1"<<std::endl;
            return;
        }
        myfile << "ИНН участника оборота,ИНН производителя,ИНН собственника,Дата производства,Тип производственного заказа,Версия" << endl;
        myfile << "1111111111,2222222222,3333333333,2019-01-01,Собственное производство,4" << endl;
        myfile << "КИ,КИТУ,Дата производства,Код ТН,ВЭД,ЕАС товара,Вид документа подтверждающего соответствие,Номер документа подтверждающего соответствие,Дата документа подтверждающего соответствие,Идентификатор ВСД" << endl;

        qDebug() << "sdfsdfds222";
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
                //result.
                auto mod_time = result.st_mtime;
                //auto ddd = result.st_mtimensec;
                cout << path_curr << " " << mod_time << endl;

                if(mod_time > max) {
                    max = mod_time;
                    maxPath = path_curr;
                }

                //cout << ddd << endl;
                //cout << file.path() << " " << result.st_mtimensec << endl;
                //...
            }
        }

        cout << "maxPath: " << maxPath << " " << max << endl;

//    auto filename = path;
//    struct stat result;
//    if(stat(filename.c_str(), &result)==0)
//    {
//        //auto mod_time = result.st_mtime;
//        cout << result.st_mtimensec << endl;
//        //...
//    }

    myfile.open (maxPath, std::ios_base::app);
    if(not myfile.is_open()) {
        std::cout<<"Error open file2"<<std::endl;
        return;
    }

//    myfile << "0104630033880100211AREwAwLETM7g240ffd0,2019-01-01,6401921000,Сертификат соответствия,123,2019-01-01,9CDA-5D57-FAEA-46DD-B94D-3DCC-AC70-1091" << endl;
    myfile << barCode << "," << buffer <<",6401921000,Сертификат соответствия,123,2019-01-01," << currentPosition.vsd << endl;

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
    auto it = positions.find(arg1);
    if(it != positions.end()) {
        currentPosition = it->second;
        ui->label_6->setNum(currentPosition.expected);
        ui->label_7->setNum(currentPosition.current);
        qDebug() << "currentPosition: " << currentPosition;
    } else {
        qDebug() << "не найдено соответствие";
    }
}

void Dialog::on_pushButton_clicked()
{
    if(STATUS_OK != sel.Open())
        qDebug() << "Corescanner service is not Active.";

    sel.GetScanners();
}
