#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <tuple>

#include "pugixml.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    cout << __PRETTY_FUNCTION__ << " start =======================" << endl;
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);

    // XML open
    pugi::xml_document doc;
    if (doc.load_file("positions.xml")) {
        cout << "Удалось загрузить XML документ" << endl;
    } else {
        cout << "Не удалось загрузить XML документ" << endl;
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

    // update vsd
    const string vsd_path = string("/mnt/hgfs/shared_folder/") + string("vsd.csv");

    const auto& vsd_per_names = get_vsds(vsd_path);
    if(vsd_per_names.empty()) {
        cout << "ВСД не найдены" << endl;
        throw std::logic_error("error");
        close();
    }

    for(auto &[name_in_vsd_file, vsd] : vsd_per_names) {
        pugi::xml_node positions_xml = doc.child("resources").child("input");

        for (pugi::xml_node position_xml: positions_xml.children("position")) {
            std::string name_in_xml = position_xml.attribute("name_english").as_string();
            if(name_in_vsd_file == name_in_xml) {
                position_xml.attribute("vsd").set_value(vsd.c_str());
                cout << "name_in_vsd_file: " << name_in_vsd_file << endl;
                cout << "set vsd to xml: " << vsd << endl;
            }
        }
    }

    if(not doc.save_file("positions.xml")) {
        cout << "Не удалось обновить XML документ" << endl;
        throw std::logic_error("error");
    } else {
        cout << "Удалось обновить XML документ" << endl;
    }

    cout << __PRETTY_FUNCTION__ << " end =======================" << endl;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked() {
    cout << __PRETTY_FUNCTION__ << " start =======================" << endl;

    string name = ui->comboBox->currentText().toUtf8().constData();

    cout << "name: " << name << endl;
    cout << "mode: " << mode << endl;
    cout << "new_template: " << new_template << endl;

    // Получить current
    update_current_in_xml(name, new_template, mode);

    std::unique_ptr<IPos> pos_handler;

    // Создать хэндлер
    switch (mode) {
    case vvod:
        pos_handler.reset(new InputPos(name));
        break;
    case vivod:
        pos_handler.reset(new OutputPos(name));
        break;
    default:
        cout << "bad mode: " << mode << endl;
        throw std::logic_error("error");
    }

    f1 = new Dialog(std::move(pos_handler), this);

    cout << __PRETTY_FUNCTION__ << " end =======================" << endl;

    hide();
    f1->exec();
}
void MainWindow::on_pushButton_2_clicked()
{
    mode = WF::vvod;
}

void MainWindow::on_pushButton_5_clicked()
{
    mode = WF::vivod;
}

void MainWindow::on_pushButton_3_clicked()
{
    new_template = true;
}

void MainWindow::on_pushButton_6_clicked()
{
    new_template = true;
}

void MainWindow::on_pushButton_4_clicked()
{
    new_template = false;
}

void MainWindow::on_pushButton_7_clicked()
{
    new_template = false;
}
