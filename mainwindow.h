#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dialog.h"

#include "sender.h"

#include "ConsoleSampleEventListener.h"

#include <utility>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_7_clicked();

private:
    void update_current_in_xml (const std::string & name, bool new_template, const WF work_format) {
        cout << __PRETTY_FUNCTION__ << " start =======================" << endl;

        cout << "try to open: " << "positions.xml" << endl;
        pugi::xml_document doc;
        if (doc.load_file("positions.xml")) {
            cout << "Удалось загрузить positions.xml документ" << endl;
        } else {
            cout << "Не удалось загрузить positions.xml документ" << endl;
            throw std::logic_error("error");
            close();
        }

        std::string mode;
        switch (work_format) {
        case vvod:
            mode = "input";
            break;
        case vivod:
            mode = "output";
            break;
        default:
            cout << "bad work_format: " << work_format << endl;
            throw std::logic_error("error");
        }

        cout << "mode=" << mode << endl;
        cout << "name=" << name << endl;

        pugi::xml_node positions_xml = doc.child("resources").child(mode.c_str());

        for (pugi::xml_node position_xml: positions_xml.children("position")) {
            std::string name_english = position_xml.attribute("name_english").as_string();

            if(name != name_english) {
                cout << "name != name_english" << endl;
                continue;
            }
            cout << "name == name_english" << endl;

            if(new_template) {
                cout << "in new_template" << endl;
                position_xml.attribute("current").set_value(0);

                if(not doc.save_file("positions.xml")) {
                    cout << "Не удалось сохранить XML документ" << endl;
                    throw std::logic_error("error");
                }

                return;
            }

            cout << "in not new_template" << endl;

            ulong max = 0;
            std::string maxPath = "";


            cout << "Шаблоны этой позиции:" << endl;
            for (const auto & file : std::filesystem::directory_iterator(std::filesystem::path("/mnt/hgfs/shared_folder/") / mode)) {
                std::string path_curr = file.path();

                name_english.append(".csv");

                std::size_t found = path_curr.find(name_english);
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
            if(maxPath.empty()) {
                cout << "Ошибка! Не найдено ни одного шаблона" << endl;
                throw std::logic_error("error");
            }

            std::ifstream ifs(maxPath);
            int numLines = 0;
            std::string unused;
            while ( std::getline(ifs, unused) )
               ++numLines;

            int current = numLines - 3;
            ifs.close();
            cout << "current=" << current << endl;
            cout << position_xml.attribute("current").set_value(current);

            if(not doc.save_file("positions.xml")) {
                cout << "Не удалось сохранить XML документ" << endl;
                throw std::logic_error("error");
            } else {
                cout << "Удалось сохранить XML документ" << endl;
                return;
            }
        }

        throw std::logic_error("error");
        cout << __PRETTY_FUNCTION__ << " end =======================" << endl;
    }

    std::string position_path;
    std::string shared_folder;
    WF mode = WF::vvod;
    bool new_template = true;

    Ui::MainWindow *ui;
    Dialog *f1;
};
#endif // MAINWINDOW_H
