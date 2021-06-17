#include "sender.h"

using namespace std;

Sender::Sender(QObject *parent) : QObject(parent) {}

void Sender::emit_code(std::string code) {
    emit emitting(QString::fromUtf8(code.c_str()));
}
