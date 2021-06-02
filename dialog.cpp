#include "dialog.h"
#include "ui_dialog.h"

#include <QMainWindow>
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QDomDocument>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <fstream>
#include <regex>

#include <filesystem>

#include <sys/types.h>
#include <sys/stat.h>

namespace fs = std::filesystem;
using namespace std::chrono_literals;
using namespace std;
using namespace std::filesystem;
using std::filesystem::directory_iterator;

ostream& operator<<( ostream&  out, Position& pos ) {
    out << "code_tn_ved: " << pos.code_tn_ved << " " << endl
        << "document_type: " << pos.document_type << " " << endl
        << "document_number: " << pos.document_number << " " << endl
        << "document_date: " << pos.document_date << " " << endl
        << "vsd: " << pos.vsd << " " << endl
        << "expected: " << pos.expected << " " << endl
        << "current: " << pos.current << " " << endl
        << "name: " << pos.name << " " << endl
        << "inn:" << pos.inn << endl;

    return out;
}

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

Dialog::Dialog(WF work_format_, bool new_template, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    sel(*this),
    work_format(work_format_)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);

    cout << "work_format: " << work_format << endl;

    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
        ui->comboBox_2->addItem(info.portName());

    const string vsd_path = string("/mnt/hgfs/shared_folder/") + string("vsd.csv");
    const string positions_path = "positions.xml";

    const auto& vsds = get_vsds(vsd_path);
    if(vsds.empty()) {
        cout << "ВСД не найдены" << endl;
        close();
    }

    if(not update_config_file(positions_path, vsds, new_template)) {
        cout << "Не получилось обновить XML конфигурационный файл" << endl;
        close();
    }

    switch (work_format) {
    case vvod:
        pos_handler.reset(new InputPos(positions_path));
        break;
    case vivod:
        pos_handler.reset(new OutputPos(positions_path));
        break;
    default:
        cout << "bad work_format: " << work_format << endl;
        return;
    }

    auto names = pos_handler->names();
    if(names.empty()) {
        cout << "Отсутсвуют считанные имена позиций" << endl;
        close();
    }

    QStringList qnames;

    for(auto const& name: names)
         qnames.push_back(QString::fromStdString(name));

    ui->comboBox->addItems(qnames);
}

void Dialog::listen_comport() {
    QSerialPort serialPort;

    //const auto serialPortInfos = QSerialPortInfo::availablePorts();
    const QString serialPortName = ui->comboBox_2->currentText();

    cout << "Comport. Port name: " << serialPortName.toUtf8().constData() << endl;
    serialPort.setPortName(ui->comboBox_2->currentText());
    serialPort.setBaudRate(57600);
    if (!serialPort.open(QIODevice::ReadOnly)) {
        cout << "Comport. FAIL open: " << serialPortName.toUtf8().constData() << " serialPort.error(): " << serialPort.error() << endl;
        return;
    } else {
        cout << "Comport. SUCCESS open" << endl;
    }

    while(not stop_listen_comport) {
        QByteArray readData = serialPort.readAll();
        while (serialPort.waitForReadyRead(400))
            readData.append(serialPort.readAll());

        if (serialPort.error() == QSerialPort::ReadError) {
            cout << "Comport. Read error: " << serialPort.errorString().toUtf8().constData() << endl;
            continue;
        } else if (serialPort.error() == QSerialPort::TimeoutError && readData.isEmpty()) {
            //cout << "Comport. No data in comport for 1sec" << endl;
            continue;
        }
        cout << "Comport. Data received: " << QString(readData).toUtf8().constData() << endl;

        std::string time_pattern = std::string("%H:%M:%S");
        time_t t = time(0);
        struct tm * now = localtime( & t );

        char time_buffer [80];
        strftime (time_buffer,80,time_pattern.c_str(),now);

        typedef std::chrono::milliseconds ms;
        cout << "time differece: " << timer.elapsed() << " ms" << endl;
        if(timer.elapsed() > 1000)
            ui->textEdit->append(QString::fromUtf8("<p style='color: red'> %1 Внимание, КМ не считан!</p>").arg(time_buffer));
        else
            ui->textEdit->append(QString::fromUtf8("<p style='color: green'> %1 Сканирование прошло успешно!</p>").arg(time_buffer));
    }
}

Dialog::~Dialog()
{
    if(comport_listener.joinable()) {
        stop_listen_comport = true;
        comport_listener.join();
    }

    sel.Close();
    delete ui;
}

bool Dialog::codeExists(std::ifstream & myfile, const string & barCode) {
    cout << "codeExists: " << endl;
    cout << "barCode: " << barCode << endl;
    int pos_to_compare = 25;
    string bar_code_to_compare = barCode.substr(0, pos_to_compare);
    cout << "bar_code_to_compare: " << bar_code_to_compare << endl;
    cout << "codes from file: " << endl;

    string line;

    for (int i = 0; std::getline(myfile, line); ++i) {
        if(i < 3)
            continue;

        string code = line.substr(0, pos_to_compare);
        cout << "line: " << line << endl;
        cout << "code: " << code << endl;

        if(code == bar_code_to_compare) {
            cout << "дубль кода: " << code << endl;
            return true;
        } else {
            cout << "оригинальный: " << code << endl;
        }
    }

    return false;
}

void Dialog::barCodeEvent(string bar_code)
{
    std::string filename_pattern = std::string("%Y-%m-%d:%H-%M");
    std::string time_pattern = std::string("%H:%M:%S");

    time_t t = time(0);
    struct tm * now = localtime( & t );

    char filename_buffer [80];
    char time_buffer [80];
    strftime (filename_buffer,80,filename_pattern.c_str(),now);
    strftime (time_buffer,80,time_pattern.c_str(),now);

    char gs = 29;

    auto pos = bar_code.find(gs);

    if(bar_code.size() <= 20) {
        ui->textEdit->append(QString::fromUtf8("<p style='color: red'> %1 Считан неверны штрих код. Повторить сканирование</p>").arg(time_buffer));
        cout << "Не верный код! Меньше 21 символов" << endl;
        return;
    }

    if(pos == string::npos)
        cout << "не найден" << endl;
    else
        cout << "gs найден: " << pos << endl;

    bar_code = bar_code.substr(0,pos);
    cout << "bar_code substr= " << bar_code << endl;


    if(pos_handler->current() == pos_handler->expected()) {
        cout << "barCodeEvent: " << "pos_handler->current() == pos_handler->expected()" << endl;
        ui->textEdit->append(QString::fromUtf8("<p style='color: red'> %1 Шаблон завершен. Скан не будет сохранен!</p>").arg(time_buffer));
        //close();
        return;
    }
    cout << "barCodeEvent: " << "pos_handler->current() != pos_handler->expected()" << endl;

    create_directories("/mnt/hgfs/shared_folder/" + pos_handler->mode());

    cout << endl << "Сохранение скана для текущей позиции: " << endl << pos_handler->to_string() << endl;

    string filename = std::string(filename_buffer) + " " + pos_handler->name() + ".csv";
    if (pos_handler->current() == 0 || pos_handler->current() == 1500) {
         std::ofstream myfile_shared;

         string myfile_shared_name = "/mnt/hgfs/shared_folder/" + pos_handler->mode() + "/" + filename;
         myfile_shared.open(myfile_shared_name);
         if(not myfile_shared.is_open()) {
             std::cout<<"Ошибка создания файла шаблона"<<std::endl;
             return;
         }

         pos_handler->write_header(myfile_shared);

         cout <<  "Создан новый шаблон для этой позиции: " << filename << endl;
         myfile_shared.close();
    }

    ulong max = 0;
    std::string maxPath = "";
    std::string maxPathFileName = "";

    cout << "Шаблоны этой позиции:" << endl;
    for (const auto & file : directory_iterator(std::filesystem::path("/mnt/hgfs/shared_folder/") / pos_handler->mode())) {
        std::string path_curr = file.path();

        std::string curName = pos_handler->name();

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

    maxPathFileName = maxPath.substr(maxPath.find_last_of("/") + 1);
    cout << "С именем: " << maxPathFileName << endl;

    bar_code = std::regex_replace(bar_code, std::regex("\""), "\"\"");
    bar_code.insert(0, 1, '"');
    bar_code.push_back('"');

    std::ifstream ifs(maxPath);
    if(codeExists(ifs, bar_code)) {
        ifs.close();
        ui->textEdit->append(QString::fromUtf8("<p style='color: red'> %1 Дубликат!</p>").arg(time_buffer));
        cout << "Дубль скана не сохранен" << endl;
        timer.reset();
        return;
    } else {
        ifs.close();
    }

    std::string output_file_name = "/mnt/hgfs/shared_folder/" + pos_handler->mode() + "/" + maxPathFileName;
    pos_handler->write_scan(output_file_name, bar_code);

    ui->label_7->setNum(pos_handler->current());

    ui->textEdit->append(QString::fromUtf8("<p style='color: green'> %1 Cчитано успешно</p>").arg(time_buffer));
    timer.reset();
    cout << "Сохранение скана для текущей позиции завершено успешно" << endl;

    if(pos_handler->current() == pos_handler->expected()) {
        QMessageBox msgBox;
        msgBox.setText("Формирование шаблона завершено");
        msgBox.setInformativeText("Плановое количество КМ достигнуто. Программа будет закрыта");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        close();
    }
}

void Dialog::on_comboBox_currentTextChanged(const QString &arg1)
{
    if(pos_handler->set_current_position_if_exists(arg1.toUtf8().constData())) {
        ui->lineEdit->setText(QString::number(pos_handler->expected()));
        ui->label_7->setNum(pos_handler->current());
    }
}

void Dialog::on_pushButton_clicked()
{
    if(not comport_listener.joinable())
        comport_listener = std::thread(&Dialog::listen_comport, this);

    if(STATUS_OK != sel.Open())
        qDebug() << "Corescanner service is not Active.";

    sel.GetScanners();
}

void Dialog::on_lineEdit_textChanged(const QString &arg1)
{
    pos_handler->expected() = arg1.toInt();
    pos_handler->update_xml("positions.xml");
}
