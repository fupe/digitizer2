// modbusworker.h
#pragma once
#include "idatasourceworker.h"
#include "settings.h"

// Pozn.: později může používat QModbusRtuSerialMaster/QModbusTcpClient
class ModbusWorker : public IDataSourceWorker {
    Q_OBJECT
public:
    explicit ModbusWorker(const ModbusParams& params, QObject* parent=nullptr)
        : IDataSourceWorker(parent), params_(params) {}

public slots:
    void open() override {
        // TODO: inicializace klienta + periodické čtení registrů/drátování dotazů
        emit errorOccured(QStringLiteral("ModbusWorker ještě není implementován."));
        emit closed();
    }
    void close() override { emit closed(); }
    void send(const QByteArray&) override { /* neaplikovatelné */ }

private:
    ModbusParams params_;
};
