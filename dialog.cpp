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
#include <thread>
#include <chrono>

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
        << "name: " << pos.name << " " << endl
        << "inn:" << pos.inn << endl
        << "expected: " << pos.expected << endl
        << "current: " << pos.current << endl;

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
        << pos.name  << " " << endl
        << "expected: " << pos.expected << endl
        << "current: " << pos.current << endl;

    return out;
}

Dialog::Dialog(std::unique_ptr<IPos> pos_handler_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    pos_handler(std::move(pos_handler_)),
    km_number(pos_handler->current()),
    sel(sender) {
    cout << __PRETTY_FUNCTION__ << " start =======================" << endl;
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

    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
        ui->comboBox_2->addItem(info.portName());

    ui->lineEdit->setText(QString::number(pos_handler->expected()));

    ui->label_6->setText(QString::fromStdString(pos_handler->name()));
    ui->label_7->setText(QString::number(pos_handler->current()));

    QObject::connect(&sender,&Sender::emitting,this,&Dialog::barCodeEvent);

    cout << __PRETTY_FUNCTION__ << " end =======================" << endl;

//    std::thread t = std::thread([&]{
//        using namespace std::chrono_literals;
//        std::this_thread::sleep_for(5s);
//        this->barCodeEvent("243543453453ewrdsfdsf33");
//        std::this_thread::sleep_for(2s);
//        this->barCodeEvent("243543453453ewrdsfdsf32");
//        std::this_thread::sleep_for(1s);
//        this->barCodeEvent("243543453453ewrdsfdsf31");
//    });
//    t.detach();
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

bool Dialog::codeExists(std::string & maxPath, const string & barCode) {
    std::ifstream myfile(maxPath);
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

    myfile.close();
    return false;
}

void Dialog::barCodeEvent(QString bar_code_)
{
    string bar_code = bar_code_.toUtf8().constData();
    string bar_code_origin = bar_code;
    cout << __PRETTY_FUNCTION__ << " start =======================" << endl;

    // Получение текущего времени в строке
    std::string filename_pattern = std::string("%Y-%m-%d %H-%M");
    std::string time_pattern = std::string("%H:%M:%S");

    time_t t = time(0);
    struct tm * now = localtime( & t );

    char filename_buffer [80];
    char time_buffer [80];
    strftime (filename_buffer,80,filename_pattern.c_str(),now);
    strftime (time_buffer,80,time_pattern.c_str(),now);

    // Валидация по 20 символов
    if(bar_code.size() <= 20) {
        ui->textEdit->append(QString::fromUtf8("<p style='color: red'> %1 Считан неверны штрих код. Повторить сканирование</p>").arg(time_buffer));
        cout << "Не верный код! Меньше 21 символов" << endl;
        return;
    }

    // Обрезание по ГС
    char gs = 29;
    auto pos = bar_code.find(gs);

    if(pos == string::npos)
        cout << "не найден" << endl;
    else
        cout << "gs найден: " << pos << endl;

    bar_code = bar_code.substr(0,pos);
    cout << "bar_code substr= " << bar_code << endl;

    // Проверка на конец программе
    if(pos_handler->current() == pos_handler->expected()) {
        cout << "barCodeEvent: " << "current == expected" << endl;
        ui->textEdit->append(QString::fromUtf8("<p style='color: red'> %1 Шаблон завершен. Скан не будет сохранен!</p>").arg(time_buffer));
        //close();
        return;
    }
    cout << "barCodeEvent: " << "pos_handler->current() != pos_handler->expected()" << endl;

    cout << endl << "Сохранение скана для текущей позиции: " << endl << pos_handler->to_string() << endl;

    create_directories(shared_folder + pos_handler->mode());
    create_directories(shared_folder + "ki/");

    // Создание шаблонов
    string filename = std::string(filename_buffer) + " " + pos_handler->name() + ".csv";
    if (pos_handler->current() == 0 || pos_handler->current() == 1500) {
         std::ofstream myfile_shared;

         string myfile_shared_name = shared_folder + pos_handler->mode() + "/" + filename;
         myfile_shared.open(myfile_shared_name);
         if(not myfile_shared.is_open()) {
             std::cout<<"Ошибка создания файла шаблона"<<std::endl;
             return;
         }

         pos_handler->write_header(myfile_shared);

         cout <<  "Создан новый шаблон для этой позиции: " << filename << endl;
         myfile_shared.close();


         string ki_file_name = std::string(filename_buffer) + " " + pos_handler->name() +  "_ki" + ".csv";
         string my_ki_file_shared_name = shared_folder + "ki/" + ki_file_name;
         std::ofstream ki_file(my_ki_file_shared_name);

         cout <<  "Создан новый шаблон КИ для этой позиции: " << ki_file_name << endl;
         ki_file.close();
    }

    ulong max = 0;
    std::string maxPath = "";
    std::string maxPathFileName = "";

    cout << "Шаблоны этой позиции:" << endl;
    // Поиск самого последнего шаблона
    for (const auto & file : directory_iterator(std::filesystem::path(shared_folder) / pos_handler->mode())) {
        std::string path_curr = file.path();

        std::string curName = pos_handler->name();
        cout << "curName=" << curName << endl;

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

    // Поиск самого последнего шаблона КИ
    std::string maxPathKi = "";
    max = 0;
    //=================================
    cout << "Шаблоны ки этой позиции:" << endl;
    for (const auto & file : directory_iterator(std::filesystem::path(shared_folder + "ki/"))) {
        std::string path_curr = file.path();

        std::string curName = pos_handler->name();
        cout << "curName=" << curName << endl;

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
                maxPathKi = path_curr;
            }
        }
    }

    cout << "Выбран самый последний из шаблонов КИ для данной позиции: " << maxPathKi << endl;


    // Проверка дубля
    bar_code = std::regex_replace(bar_code, std::regex("\""), "\"\"");
    bar_code.insert(0, 1, '"');
    bar_code.push_back('"');


    if(codeExists(maxPath, bar_code)) {
        ui->textEdit->append(QString::fromUtf8("<p style='color: red'> %1 Дубликат!</p>").arg(time_buffer));
        auto p = ui->textEdit->textCursor();
        p.movePosition(QTextCursor::End);
        ui->textEdit->setTextCursor(p);
        cout << "Дубль скана не сохранен" << endl;
        timer.reset();
        return;
    }

    // Запись скана
    std::string output_file_name = shared_folder + pos_handler->mode() + "/" + maxPathFileName;

    pos_handler->write_scan(output_file_name, bar_code);

    // Запись КИ. Выполнить после write_scan! Т.к. там внутри инкремент current.
    std::ofstream ki_file;

    ki_file.open (maxPathKi, std::ios_base::app);
    if(not ki_file.is_open()) {
        std::cout<<"Ошибка открытия шаблона для данной позиции" << std::endl;
        return;
    }

    ki_file << bar_code_origin;

    if(pos_handler->current() != pos_handler->expected())
        ki_file << endl;

    ki_file.close();

    cout << "КИ записан" << endl;

    // Оповещение UI

    ui->label_7->setNum(pos_handler->current());

    km_number++;
    ui->textEdit->append(QString::fromUtf8("<p style='color: green'> %1 КМ считан успешно %2</p>").arg(time_buffer).arg(km_number));
    auto p = ui->textEdit->textCursor();
    p.movePosition(QTextCursor::End);
    ui->textEdit->setTextCursor(p);

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

    cout << __PRETTY_FUNCTION__ << " end =======================" << endl;
}

void Dialog::on_pushButton_clicked()
{
    if(not comport_listener.joinable())
        comport_listener = std::thread(&Dialog::listen_comport, this);

    if(STATUS_OK != sel.Open())
        qDebug() << "Corescanner service is not Active.";

    sel.GetScanners();
    ui->lineEdit->setEnabled(false);
    update_expected(ui->lineEdit->text().toInt());
}

void Dialog::update_expected(int expected) {
    cout << __PRETTY_FUNCTION__ << " start =======================" << endl;
    std::string xml_path = "positions.xml";

    pugi::xml_document doc;

    if (not doc.load_file(xml_path.c_str())) {
        cout << "Не удалось загрузить xml_path.c_str()) документ" << xml_path.c_str() << endl;
        throw std::logic_error("error");
    } else {
        cout << "Удалось загрузить xml_path.c_str()) документ" << xml_path.c_str() << endl;
    }

    pugi::xml_node positions_xml = doc.child("resources").child(pos_handler->mode().c_str());
    cout << "mode: " << pos_handler->mode() << endl;
    cout << "name: " << pos_handler->name() << endl;

    for (pugi::xml_node position_xml: positions_xml.children("position")) {
        std::string position_name = position_xml.attribute("name_english").as_string();
        if(position_name == pos_handler->name()) {
            cout << "set expected number: " << expected << endl;
            position_xml.attribute("expected").set_value(expected);
            pos_handler->set_expected(expected);
        }
    }

    if(not doc.save_file(xml_path.c_str())) {
        cout << "Не удалось сохранить XML документ" << endl;
        return;
    }
    cout << "Удалось сохранить XML документ" << endl;

    cout << __PRETTY_FUNCTION__ << " end =======================" << endl;
}
