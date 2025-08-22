// idatasourceworker.h
#pragma once
#include <QObject>
#include <QByteArray>
#include "settings.h"

// Jednotný formát přijatého rámce/dat:
struct Frame {
    QByteArray data;
    qint64 ts_msec;  // timestamp v ms od epochy
};

class IDataSourceWorker : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~IDataSourceWorker() = default;

public slots:
    // otevře spojení dle nastavení předaného v setXxx předem
    virtual void open() = 0;
    virtual void close() = 0;

    // odesílání (pokud zdroj podporuje)
    virtual void send(const QByteArray& bytes) = 0;

signals:
    void opened();
    void closed();
    void errorOccured(const QString& msg);
    void frameReceived(const Frame& f);
};
