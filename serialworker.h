// serialworker.h
#pragma once
#include "idatasourceworker.h"
#include <QSerialPort>
#include <QTimer>

class SerialWorker : public IDataSourceWorker {
    Q_OBJECT
public:
    explicit SerialWorker(const SerialParams& params, QObject* parent=nullptr);


public slots:
    void open() override;
    void close() override;
    void send(const QByteArray& bytes) override;

private slots:
    void onReadyRead();

private:
    QSerialPort port_;
    SerialParams params_ ;
    QByteArray lineBuffer_; // perzistentní buffer do nejbližšího '\n'
};
