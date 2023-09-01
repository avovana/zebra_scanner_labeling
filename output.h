#pragma once

#include <QDialog>
#include <QDebug>
#include <QDomDocument>

#include "position.h"

namespace Ui {
class Output;
}

class Output : public QDialog
{
    Q_OBJECT

public slots:
    void barCodeEvent(QString bar_code_);

public:
    explicit Output(std::unique_ptr<IPos> pos_handler_, QWidget *parent = nullptr);
    ~Output();


    QString get_decode_data(std::string outXml);
    std::vector<std::string> stringTokernize(std::string inStr, char cDelim);
    bool codeExists(string &maxPath, const string &barCode);
    void listen_comport();
    void update_expected(int);
private slots:
    void on_pushButton_clicked();
private:
    Ui::Output *ui;

    std::chrono::time_point<std::chrono::high_resolution_clock> last_scan_time;
    std::thread comport_listener;
    bool stop_listen_comport = false;
    Timer timer;
    std::unique_ptr<IPos> pos_handler;
    int km_number = 0;

    std::string position_path;
    std::string shared_folder;

    Sender sender;
    SampleEventListener sel;
};
