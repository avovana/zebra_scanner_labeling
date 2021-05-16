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

Dialog::Dialog(WF work_format_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    sel(*this),
    work_format(work_format_)
{
    ui->setupUi(this);

    cout << "work_format: " << work_format << endl;

    switch (work_format) {
    case vvod:
        pos_handler.reset(new InputPos());
        break;
    case vivod:
        pos_handler.reset(new OutputPos());
        break;
    default:
        cout << "bad work_format: " << work_format << endl;
        return;
    }

    comport_listener = std::thread(&Dialog::listen_comport, this);

    pugi::xml_document doc;
    if (!doc.load_file("positions.xml")) {
        cout << "Не удалось загрузить XML документ" << endl;
        return;
    } else {
        cout << "Удалось загрузить XML документ" << endl;
    }

    auto names = pos_handler->add_positions(doc);
    if(names.empty())
        return;

    QStringList qnames;
    qnames.reserve(names.size());

    for(auto const& name: names)
         qnames.push_back(QString::fromStdString(name));

    ui->comboBox->addItems(qnames);
}

void Dialog::listen_comport() {
    QSerialPort serialPort;

    const auto serialPortInfos = QSerialPortInfo::availablePorts();

    const QString serialPortName = serialPortInfos.at(1).portName();
    cout << "Comport. Port name: " << serialPortName.toUtf8().constData() << endl;
    serialPort.setPortName(serialPortName);
    serialPort.setBaudRate(57600);
    if (!serialPort.open(QIODevice::ReadOnly)) {
        cout << "Comport. FAIL open: " << serialPortName.toUtf8().constData() << " serialPort.error(): " << serialPort.error() << endl;
        return;
    } else {
        cout << "Comport. SUCCESS open" << endl;
    }

    while(not stop_listen_comport) {
        QByteArray readData = serialPort.readAll();
        while (serialPort.waitForReadyRead(30))
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
        if(timer.elapsed() > 1000) {
            ui->textEdit->append(QString::fromUtf8("<p style='color: red'> %1 За последнюю 1 секунду не было скана!</p>").arg(time_buffer));
        } else {
            ui->textEdit->append(QString::fromUtf8("<p style='color: green'> %1 Отсканированный товар прошел!</p>").arg(time_buffer));
        }
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

bool Dialog::codeExists(std::ifstream & myfile, string & barCode) {

    codes.clear();
    string line;
    for (int i = 0; std::getline(myfile, line); ++i) {
        if(i < 3)
            continue;

        cout << "line: " << line << endl;
        string code = line.substr(1, line.find(",") - 2);

        cout << "code: " << code << endl;

        codes.insert(code);
    }

    return codes.count(barCode);
}

void Dialog::barCodeEvent(string barCode)
{
    std::string filename_pattern = std::string("%Y-%m-%d:%H-%M");
    std::string time_pattern = std::string("%H:%M:%S");

    time_t t = time(0);
    struct tm * now = localtime( & t );

    char filename_buffer [80];
    char time_buffer [80];
    strftime (filename_buffer,80,filename_pattern.c_str(),now);
    strftime (time_buffer,80,time_pattern.c_str(),now);

    fs::create_directories(pos_handler->mode()); // don't need to always create

    std::ofstream myfile;

    cout << endl << "Сохранение скана для текущей позиции: " << endl << pos_handler->to_string() << endl;

    string filename = std::string(filename_buffer) + " " + pos_handler->name() + ".csv";
    if(pos_handler->current() == 0 || pos_handler->current() == 1500) {
        std::filesystem::path cwd = std::filesystem::current_path();
        cwd /= pos_handler->mode();
        std::filesystem::path filePath = cwd / filename;

        myfile.open(filePath);
        if(not myfile.is_open()) {
            std::cout<<"Ошибка создания файла шаблона"<<std::endl;
            return;
        }

        pos_handler->write_header(myfile);

        cout <<  "Создан новый шаблон для этой позиции: " << filename << endl;
        myfile.close();
    }


    using std::cout; using std::cin;
    using std::endl; using std::string;
    using std::filesystem::directory_iterator;

    using std::filesystem::current_path;

    ulong max = 0;
    std::string maxPath = "";

    cout << "Шаблоны этой позиции:" << endl;
    for (const auto & file : directory_iterator(current_path() / pos_handler->mode())) {
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

    std::ifstream ifs(maxPath);
    if(codeExists(ifs, barCode)) {
        ifs.close();
        ui->textEdit->append(QString::fromUtf8("<p style='color: red'> %1 Дубликат!</p>").arg(time_buffer));
        cout << "Дубль скана не сохранен" << endl;
        timer.reset();
        return;
    } else {
        ifs.close();
    }

    barCode = std::regex_replace(barCode, std::regex("\""), "\"\"");
    barCode.insert(0, 1, '"');
    barCode.push_back('"');

    pos_handler->write_scan(maxPath, barCode);
    pos_handler->update_xml("positions.xml");

    ui->label_7->setNum(pos_handler->current());

    ui->textEdit->append(QString::fromUtf8("<p style='color: green'> %1 Cчитано успешно</p>").arg(time_buffer));
    timer.reset();
    cout << "Сохранение скана для текущей позиции завершено успешно" << endl;
}

void Dialog::on_comboBox_currentTextChanged(const QString &arg1)
{
    if(pos_handler->set_current_position_if_exists(arg1.toUtf8().constData())) {
        ui->label_6->setNum(pos_handler->expected());
        ui->label_7->setNum(pos_handler->current());
    }
}

void Dialog::on_pushButton_clicked()
{
    if(STATUS_OK != sel.Open())
        qDebug() << "Corescanner service is not Active.";

    sel.GetScanners();
}
