#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <tuple>

#include "pugixml.hpp"
//#include "foobar_version.h"

#include "sender.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    cout << __PRETTY_FUNCTION__ << " start =======================" << endl;
    cout <<  " Vers: " << 111 << endl;

    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);

    // XML open
    pugi::xml_document doc_vars;
    if (doc_vars.load_file("vars.xml")) {
        cout << "Удалось загрузить vars.xml документ" << endl;
    } else {
        cout << "Не удалось загрузить vars.xml документ" << endl;
        throw std::logic_error("error");
        close();
    }

    pugi::xml_node position_path_xml = doc_vars.child("vars").child("position_path");
    pugi::xml_node shared_folder_xml = doc_vars.child("vars").child("shared_folder");
    position_path = position_path_xml.text().get();
    shared_folder = shared_folder_xml.text().get();
    cout << "position_path: " << position_path << endl;
    cout << "shared_folder: " << shared_folder << endl;

    // XML open
    pugi::xml_document doc;
    if (doc.load_file(position_path.c_str())) {
        cout << "Удалось загрузить position документ" << endl;
    } else {
        cout << "Не удалось загрузить position документ" << endl;
        throw std::logic_error("error");
        close();
    }

    // XML verification
    pugi::xml_node positions_xml = doc.child("resources").child("input");
    //pugi::xml_node positions_xml = doc.child("resources").child("output"); // тоже верифицировать
    if (positions_xml.children("position").begin() == positions_xml.children("position").end()) {
        cout << "ОШИБКА! В документе нет позиций!" << endl;
        throw std::logic_error("error");
    }

    // XML -> names
    cout << positions_xml << endl;
    for (pugi::xml_node position_xml: positions_xml.children("position")) {
        std::string name_in_xml = position_xml.attribute("name_english").as_string();
        ui->comboBox->addItem(QString::fromStdString(name_in_xml));
    }

    if(not doc.save_file("positions.xml")) {
        cout << "Не удалось сохранить positions.xml документ" << endl;
        return;
    }
    cout << "удалось сохранить positions.xml документ" << endl;
    cout << __PRETTY_FUNCTION__ << " end =======================" << endl;
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_pushButton_clicked() {
    cout << __PRETTY_FUNCTION__ << " start =======================" << endl;

    cout << "111: " << endl;
    string name = ui->comboBox->currentText().toUtf8().constData();

    cout << "name: " << name << endl;
    cout << "mode: " << mode << endl;
    cout << "new_template: " << new_template << endl;

    //=======================================

//    if(new_template) {
//    }

    // Получить current
    update_current_in_xml(name, new_template, mode);

    std::unique_ptr<IPos> pos_handler;

    // Создать хэндлер
    switch (mode) {
    case vvod:
        pos_handler.reset(new InputPos(name));
        input_ui_ = new Dialog(std::move(pos_handler), this);
        input_ui_->exec();
        break;
    case vivod:
        pos_handler.reset(new OutputPos(name));
        output_ui_ = new Output(std::move(pos_handler), this);
        output_ui_->exec();
        break;
    default:
        cout << "bad mode: " << mode << endl;
        throw std::logic_error("error");
    }

    hide();
    cout << __PRETTY_FUNCTION__ << " end =======================" << endl;
}
void MainWindow::on_pushButton_2_clicked()
{
    mode = WF::vvod;
}

void MainWindow::on_pushButton_5_clicked()
{
    mode = WF::vivod;
}

void MainWindow::on_pushButton_3_clicked() {
    ui->pushButton_3->setChecked(true);
    ui->pushButton_4->setChecked(false);

    string name = ui->comboBox->currentText().toStdString();

    new_template = true;

    // XML open
    pugi::xml_document doc;
    if (doc.load_file("positions.xml")) {
        cout << "Удалось загрузить positions.xml документ"  << endl;
    } else {
        cout << "Не Удалось загрузить positions.xml документ"  << endl;
        throw std::logic_error("error");
        close();
    }

    pugi::xml_node positions_xml = doc.child("resources").child("input");

    for (pugi::xml_node position_xml: positions_xml.children("position")) {
        std::string name_in_xml = position_xml.attribute("name_english").as_string();
        std::string vsd = position_xml.attribute("vsd").as_string();
        std::cout << "  name_in_xml: " << name_in_xml << std::endl;
        if(name == name_in_xml) {
            std::cout << "  name: " << name << "= name_in_xml: " << name_in_xml << "vsd=" << vsd << std::endl;
            ui->label->setText(QString::fromStdString(vsd));
        }
    }
}

void MainWindow::on_pushButton_6_clicked()
{
    new_template = true;
}

void MainWindow::on_pushButton_4_clicked() {
    cout << __PRETTY_FUNCTION__ << " start =======================" << endl;

    ui->pushButton_4->setChecked(true);
    ui->pushButton_3->setChecked(false);

    new_template = false;

    pugi::xml_document doc;
    if (doc.load_file("positions.xml")) {
        cout << "Удалось загрузить positions.xml документ"  << endl;
    } else {
        cout << "Не Удалось загрузить positions.xml документ"  << endl;
        throw std::logic_error("error");
        close();
    }

    string name = ui->comboBox->currentText().toStdString();

    pugi::xml_node positions_xml = doc.child("resources").child("input");

    for (pugi::xml_node position_xml: positions_xml.children("position")) {
        string name_in_xml = position_xml.attribute("name_english").as_string();
        string vsd = position_xml.attribute("vsd").as_string();
        if(name == name_in_xml)
            ui->label->setText(QString::fromStdString(vsd));
    }

    cout << __PRETTY_FUNCTION__ << " end =======================" << endl;
}

void MainWindow::on_pushButton_7_clicked()
{
    new_template = false;
}
