// serialmanager.h
#pragma once
#include <QObject>
#include <QThread>
#include <QFile>
#include "idatasourceworker.h"
#include "settings.h"

// SerialManager = "facade": podle Settings vytvoří a spravuje konkrétního worker-a
class SerialManager : public QObject {
    Q_OBJECT
public:
    explicit SerialManager(QObject* parent=nullptr);
    ~SerialManager();

    void applySettings(const Settings& s);  // zavolat při změně Settings
    DataSource currentSource() const { return source_; }

public slots:
    void open();
    void close();
    void send(const QByteArray& bytes);

    // zap/vyp logování pro pozdější simulaci
    void setRecording(bool enabled, const QString& filePath);

signals:
    void opened();
    void closed();
    void errorOccured(const QString& msg);
    void frameReceived(const Frame& f);

private slots:
    void onWorkerOpened();
    void onWorkerClosed();
    void onWorkerError(const QString& msg);
    void onWorkerFrame(const Frame& f);

private:
    void rebuildWorker(); // vytvoří worker podle Settings::source
    void destroyWorker();

private:
    Settings settings_;
    DataSource source_ = DataSource::None;

    QThread workerThread_;
    IDataSourceWorker* worker_ = nullptr; // vlastněno tímto objektem (po přesunu do threadu)

    // záznam přijatých dat pro simulaci (CSV: ts;payload)
    bool recording_ = false;
    QFile recordFile_;
};
