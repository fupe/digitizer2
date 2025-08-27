// simworker.cpp
#include "simworker.h"
#include <QDateTime>
#include <QStringList>
#include <QThread>

SimWorker::SimWorker(const SimulationParams &params, QObject *parent)
    : IDataSourceWorker(parent), params_(params) {}

void SimWorker::open() {
<<<<<<< HEAD
  if (file_.isOpen())
    file_.close();
  file_.setFileName(params_.logFile);
  if (!file_.open(QIODevice::ReadOnly | QIODevice::Text)) {
=======
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
>>>>>>> pr-24
    emit errorOccured(
        QStringLiteral("Nelze otevřít log %1").arg(params_.logFile));
    return;
  }
<<<<<<< HEAD
  startWallMsec_ = QDateTime::currentMSecsSinceEpoch();
  firstTs_ = -1;
  if (!timer_) {
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &SimWorker::tick);
  }
  timer_->start(1); // rychlý tick – dávkujeme sami dle časů v souboru
  emit opened();
}

void SimWorker::close() {
  if (timer_)
    timer_->stop();
  if (file_.isOpen())
    file_.close();
  emit closed();
}

void SimWorker::tick() {

=======
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
>>>>>>> pr-24
  // čteme řádky a vydáváme je podle časové značky (ts_msec)
  while (file_.canReadLine()) {
    const QByteArray raw = file_.readLine();
    const QString s = QString::fromUtf8(raw).trimmed();
<<<<<<< HEAD
    if (s.isEmpty())
      continue;
    const QStringList parts = s.split(';');
    if (parts.size() < 2)
      continue; // špatný řádek

    bool ok = false;
    qint64 ts = parts[0].toLongLong(&ok);
    if (!ok)
      continue;
    const QByteArray payload = parts.mid(1).join(";").toUtf8();

    if (firstTs_ < 0)
      firstTs_ = ts;

    // spočítat kdy má být tento řádek vydán vzhledem k prvnímu a k speedFactor
    const qint64 rel = ts - firstTs_;
    const qint64 targetDelay =
        qint64(double(rel) / qMax(0.0001, params_.speedFactor));
    const qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - startWallMsec_;
    if (elapsed < targetDelay) {
      // ještě brzy – necháme timer doběhnout později
      return;
    }

    Frame f{payload, QDateTime::currentMSecsSinceEpoch()};
    emit frameReceived(f);
    // pokračujeme na další řádek (možné dohnání při zrychlení)
    qDebug() << "tick ";
  }
=======
    qDebug() << "čtu" << s;
    if (s.isEmpty()) {
      qDebug() << "prázdný řádek";
      continue;
    }

    const QStringList parts = s.split(';');
    if (parts.size() < 2) {
      qDebug() << "špatný řádek" << s;
      continue; // špatný řádek
    }

    bool ok = false;
    qint64 ts = parts[0].toLongLong(&ok);
    if (!ok) {
      qDebug() << "špatný timestamp" << parts[0];
      continue;
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
      qDebug() << "čekám" << (targetDelay - elapsed) << "ms";
      // ještě brzy – necháme timer doběhnout později
      return;
    }

    Frame f{payload, QDateTime::currentMSecsSinceEpoch()};
    qDebug() << "emit frame" << payload;
    emit frameReceived(f);
    // pokračujeme na další řádek (možné dohnání při zrychlení)
  }
  qDebug() << "konec souboru";
>>>>>>> pr-24
  // Konec souboru
  if (timer_)
    timer_->stop();
}
