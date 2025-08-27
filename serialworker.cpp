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
    //qDebug() << "prisly data";
    lineBuffer_.append(port_.readAll());
    //qDebug() << "SerialWorker::onReadyRead buffer" << lineBuffer_;

    // sjednoť ukončení řádků: nahraď CR za LF
    lineBuffer_.replace('\r', '\n');

    int idx;
    while ((idx = lineBuffer_.indexOf('\n')) >= 0) {
        QByteArray line = lineBuffer_.left(idx);
        lineBuffer_.remove(0, idx + 1);
        if (line.isEmpty())
            continue;
        //qDebug() << "SerialWorker frame" << line;
        // strip any binary prefix and trailing data
        int hashPos = line.indexOf('#');
        int pipePos = line.indexOf('|', hashPos + 1);
        if (hashPos >= 0) {
            if (pipePos > hashPos)
                line = line.mid(hashPos, pipePos - hashPos);
            else
                line = line.mid(hashPos);
            //qDebug() << "SerialWorker frame2" << line;

            Frame f;
            f.data = line;
            f.ts_msec = QDateTime::currentMSecsSinceEpoch(); // časová značka pro záznam/simulaci
            emit frameReceived(f);
        } else {
            qDebug() << "SerialWorker: dropping line" << line;
        }
    }
}
