// serialworker.cpp
#include "serialworker.h"
#include <QDateTime>

SerialWorker::SerialWorker(const SerialParams& params, QObject* parent)
    : IDataSourceWorker(parent), params_(params)
{
    // nic – port nastavíme až v open()
}

void SerialWorker::open() {
    if (port_.isOpen()) port_.close();

    port_.setPortName(params_.portName);
    port_.setBaudRate(params_.baudRate);
    port_.setDataBits(static_cast<QSerialPort::DataBits>(params_.dataBits));
    port_.setStopBits(params_.stopBits==2 ? QSerialPort::TwoStop : QSerialPort::OneStop);
    port_.setParity(QSerialPort::NoParity);
    port_.setFlowControl(QSerialPort::NoFlowControl);

    if (!port_.open(QIODevice::ReadWrite)) {
        emit errorOccured(QStringLiteral("Nelze otevřít %1: %2")
                          .arg(params_.portName, port_.errorString()));
        return;
    }
    connect(&port_, &QSerialPort::readyRead, this, &SerialWorker::onReadyRead);
    emit opened();
}

void SerialWorker::close() {
    if (port_.isOpen()) {
        port_.close();
        emit closed();
    }
}

void SerialWorker::send(const QByteArray& bytes) {
    if (port_.isOpen())
        port_.write(bytes);
}

void SerialWorker::onReadyRead() {
    // Čteme po částech, seskládáme na řádky (A/B/I chodí „po řádcích“)
    lineBuffer_.append(port_.readAll());
    int idx;
    while ((idx = lineBuffer_.indexOf('\n')) >= 0) {
        QByteArray line = lineBuffer_.left(idx);
        // odstraníme i \r případně
        if (!line.isEmpty() && line.endsWith('\r')) line.chop(1);
        lineBuffer_.remove(0, idx+1);

        Frame f;
        f.data = line;
        f.ts_msec = QDateTime::currentMSecsSinceEpoch(); // časová značka pro záznam/simulaci
        emit frameReceived(f);
    }
}
