// simworker.cpp
#include "simworker.h"
#include <QDateTime>
#include <QStringList>
#include <QThread>

SimWorker::SimWorker(const SimulationParams &params, QObject *parent)
    : IDataSourceWorker(parent), params_(params) {}

void SimWorker::open() {

  qDebug() << "SimWorker::open v" << QThread::currentThread();
  if (QThread::currentThread() != thread()) {
    qDebug() << "open přesměrováno do vlákna" << thread();
    QMetaObject::invokeMethod(this, &SimWorker::open, Qt::QueuedConnection);
    return;
  }

  if (file_.isOpen())
    file_.close();
  file_.setFileName(params_.logFile);
  qDebug() << "Otevírám log" << params_.logFile;
  if (!file_.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Nepovedlo se otevřít log" << params_.logFile;

    emit errorOccured(
        QStringLiteral("Nelze otevřít log %1").arg(params_.logFile));
    return;
  }
  qDebug() << "Log otevřen";
  startWallMsec_ = QDateTime::currentMSecsSinceEpoch();
  firstTs_ = -1;
  if (!timer_) {
    qDebug() << "Vytvářím timer";
    timer_ = new QTimer(this);
    timer_->moveToThread(thread());
    connect(timer_, &QTimer::timeout, this, &SimWorker::tick);
  }
  qDebug() << "Startuji timer";
  timer_->start(1); // rychlý tick – dávkujeme sami dle časů v souboru
  emit opened();
  qDebug() << "Emitováno opened";
}

void SimWorker::close() {
  qDebug() << "SimWorker::close";
  if (QThread::currentThread() != thread()) {
    qDebug() << "close přesměrováno do vlákna" << thread();
    QMetaObject::invokeMethod(this, &SimWorker::close, Qt::QueuedConnection);
    return;
  }

  if (timer_) {
    qDebug() << "Zastavuji timer";
    timer_->stop();
  }
  if (file_.isOpen()) {
    qDebug() << "Zavírám soubor";
    file_.close();
  }
  emit closed();
  qDebug() << "Emitováno closed";
}

void SimWorker::tick() {
  qDebug() << "tick";

  if (pendingLine_.isEmpty()) {
    if (file_.atEnd()) {
        qDebug() << "konec souboru – restart";
        file_.seek(0);
        startWallMsec_ = QDateTime::currentMSecsSinceEpoch();
        firstTs_ = -1;
        timer_->start(1);
      return;
    }

    // načti nový řádek a ulož si ho pro případné pozdější odeslání
    const QByteArray raw = file_.readLine();
    pendingLine_ = QString::fromUtf8(raw).trimmed();
    qDebug() << "čtu" << pendingLine_;
    if (pendingLine_.isEmpty()) {
      qDebug() << "prázdný řádek";
      timer_->start(1);
      return;
    }
  }

  const QStringList parts = pendingLine_.split(';');
  if (parts.size() < 2) {
    qDebug() << "špatný řádek" << pendingLine_;
    pendingLine_.clear();
    timer_->start(1);
    return; // špatný řádek
  }

  bool ok = false;
  qint64 ts = parts[0].toLongLong(&ok);
  if (!ok) {
    qDebug() << "špatný timestamp" << parts[0];
    pendingLine_.clear();
    timer_->start(1);
    return;
  }
  const QByteArray payload = parts.mid(1).join(";").toUtf8();

  if (firstTs_ < 0) {
    firstTs_ = ts;
    qDebug() << "první ts" << firstTs_;
  }

  // spočítat kdy má být tento řádek vydán vzhledem k prvnímu a k speedFactor
  const qint64 rel = ts - firstTs_;
  const qint64 targetDelay =
      qint64(double(rel) / qMax(0.0001, params_.speedFactor));
  const qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - startWallMsec_;
  if (elapsed < targetDelay) {
    const qint64 wait = targetDelay - elapsed;
    qDebug() << "čekám" << wait << "ms";
    // ještě brzy – restartujeme timer na zbytek zpoždění a necháme řádek v bufferu
    timer_->start(wait);
    return;
  }

  Frame f{payload, QDateTime::currentMSecsSinceEpoch()};
  qDebug() << "emit frame" << payload;
  emit frameReceived(f);
  pendingLine_.clear();
  // zkusíme hned načíst další řádek
  timer_->start(1);
}
