// simworker.h
#pragma once
#include "idatasourceworker.h"
#include <QDebug>
#include <QFile>
#include <QTimer>

// Jednoduchý přehrávač z CSV/NDJSON, kde je sloupec ts a data.
// Pro jednoduchost použijeme CSV: ts_msec;payload
class SimWorker : public IDataSourceWorker {
  Q_OBJECT
public:
  explicit SimWorker(const SimulationParams &params, QObject *parent = nullptr);

public slots:
  void open() override;
  void close() override;
  void send(const QByteArray &) override {} // simulace neodesílá

private slots:
  void tick();

private:
  SimulationParams params_;
  QFile file_;
  qint64 startWallMsec_ = 0;
  qint64 firstTs_ = -1;
  QTimer *timer_ = nullptr;
  QString pendingLine_;
};
