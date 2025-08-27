// serialmanager.cpp
#include "serialmanager.h"
#include "serialworker.h"
#include "simworker.h"
#include "modbusworker.h"
#include <QTextStream>
#include <QDebug>

SerialManager::SerialManager(QObject* parent) : QObject(parent) {
    // Vlákno držíme persistentně; workera do něj vždy přesuneme.
    workerThread_.setObjectName("DataSourceThread");
    workerThread_.start();
}

SerialManager::~SerialManager() {
    close();
    destroyWorker();
    workerThread_.quit();
    workerThread_.wait();
}

void SerialManager::applySettings(const Settings& s) {
    settings_ = s;
    if (s.datasource != source_) {
        // změna zdroje => přestavět workera
        close();
        destroyWorker();
        source_ = s.datasource;
        rebuildWorker();
    } else {
        // stejný zdroj – můžeme jen zaktualizovat parametry (pokud to worker podporuje)
        // pro jednoduchost teď necháme tak; případně by se dal worker znovu vyrobit
    }
}

void SerialManager::rebuildWorker() {
    if (worker_) return; // už existuje

    switch (source_) {
        case DataSource::Serial:
            worker_ = new SerialWorker(settings_.serial);
            break;
        case DataSource::Simulation:
            worker_ = new SimWorker(settings_.simulation);
            break;
        case DataSource::Modbus:
            worker_ = new ModbusWorker(settings_.modbus);
            break;
        case DataSource::None:
        default:
            worker_ = nullptr;
            return;
    }

    worker_->moveToThread(&workerThread_);

    // signály workera → manager
    connect(worker_, &IDataSourceWorker::opened,          this, &SerialManager::onWorkerOpened);
    connect(worker_, &IDataSourceWorker::closed,          this, &SerialManager::onWorkerClosed);
    connect(worker_, &IDataSourceWorker::errorOccured,    this, &SerialManager::onWorkerError);
    connect(worker_, &IDataSourceWorker::frameReceived,   this, &SerialManager::onWorkerFrame);

    // životní cyklus: zničení ve vlákně workera
    connect(&workerThread_, &QThread::finished, worker_, &QObject::deleteLater);
}

void SerialManager::destroyWorker() {
    if (!worker_) return;
    // zavřít a smazat ve vlákně
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
    worker_->deleteLater();
    worker_ = nullptr;
}

void SerialManager::open() {
    if (!worker_) rebuildWorker();
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_, "open", Qt::QueuedConnection);
}

void SerialManager::close() {
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
}

void SerialManager::send(const QByteArray& bytes) {
    if (!worker_) return;
    QMetaObject::invokeMethod(worker_, "send", Qt::QueuedConnection, Q_ARG(QByteArray, bytes));
}

void SerialManager::setRecording(bool enabled, const QString& filePath) {
    recording_ = enabled;
    if (recordFile_.isOpen()) recordFile_.close();
    if (recording_) {
        recordFile_.setFileName(filePath);
        if (!recordFile_.open(QIODevice::WriteOnly | QIODevice::Text)) {
            recording_ = false;
            emit errorOccured(QStringLiteral("Nelze otevřít soubor pro záznam: %1").arg(filePath));
        }
    }
}

void SerialManager::onWorkerOpened()  { emit opened(); }
void SerialManager::onWorkerClosed()  {
    setRecording(false, QString());
    emit closed();
}
void SerialManager::onWorkerError(const QString& msg) { emit errorOccured(msg); }

void SerialManager::onWorkerFrame(const Frame& f) {
    //qDebug() << "SerialManager::onWorkerFrame" << f.data;

    // pokud je zapnuté nahrávání, ulož řádek "ts;payload\n"
    if (recording_ && recordFile_.isOpen()) {
        QTextStream out(&recordFile_);
        out << f.ts_msec << ';' << QString::fromUtf8(f.data) << '\n';
        recordFile_.flush();
    }
    emit frameReceived(f);
}
