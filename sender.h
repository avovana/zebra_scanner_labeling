#pragma once

#include <QObject>

class Sender : public QObject
{
    Q_OBJECT
public:
    explicit Sender(QObject *parent = 0);
    void emit_code(std::string);

signals:
    void emitting(QString);
};
