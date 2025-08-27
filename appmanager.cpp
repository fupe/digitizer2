#include "appmanager.h"
#include <QDebug>
#include <QCoreApplication>
#include <QMetaObject>
#include <cmath>
#include "serialmanager.h"
#include "settingsmanager.h"
#include "idatasourceworker.h"

namespace {
    constexpr double kPi = 3.14159265358979323846;
}

AppManager::AppManager(QObject* parent) : QObject(parent) {
     serialmanager_ = new SerialManager(this);
     //serialmanager_->open();
     QObject::connect(qApp, &QCoreApplication::aboutToQuit, serialmanager_, &SerialManager::close);
}

/*ShapeManager& AppManager::shapeManager()
{
    return shapeManager_;
}*/

double AppManager::getAlfa()
{
    return alfa_ ;
}

double AppManager::getBeta()
{
    return beta_ ;
}

void AppManager::setAngles(double alfa, double beta, int index)
{
    endPointBefore_ = endPointArm2_;
    qDebug() << "appmanager alfa" << alfa_ << "beta" << beta_ << "index" << index;
    alfa_ = alfa;
    beta_ = beta;


    x_value_  = std::cos(alfa_ - settings_.alfa_offset) * settings_.arm1_length;
    y_value_  = std::sin(alfa_ - settings_.alfa_offset) * settings_.arm1_length;

    const double a1 = (alfa_ - settings_.alfa_offset);
    const double a2 = (beta  - settings_.beta_offset);
    const double phi = a1 - a2 - kPi;

    x_value2_ = x_value_ + std::cos(phi) * settings_.arm2_length;
    y_value2_ = y_value_ + std::sin(phi) * settings_.arm2_length;

    Arm1Angle_ = (a1 * 180.0 / kPi);
    Arm2Angle_ = ((alfa_ - settings_.alfa_offset) - (beta_ - settings_.beta_offset) - kPi) * 180.0 / kPi;

    endPointArm1_ = QPointF(x_value_,  y_value_);
    endPointArm2_ = QPointF(x_value2_, y_value2_);

    if (endPointBefore_ != endPointArm2_) {
        emit armsUpdated(Arm1Angle_, Arm2Angle_, endPointArm1_, endPointArm2_);
        qDebug() << "appmanager alfa" << alfa_ << "beta" << beta_ << "index" << index;
    }

    indexBefore_ = index;

    // vzdálenost – použijeme robustní std::hypot
    distance = std::hypot(lastpoint.x() - endPointArm2_.x(),
                          lastpoint.y() - endPointArm2_.y());

    if (distance > settings_.auto_step && currentContiMode_ == ContiMode::Continous)
    {
        qDebug() << "distance " << distance;
        if (currentAddPointMode_ == AddPointMode::None) {
            addPointtoShapeManager();
        }
        if (currentAddPointMode_ == AddPointMode::Polyline) {
            // zde případně doplnit přidání do polyline
        }
        lastpoint = endPointArm2_;
    }

    emit sceneModified(endPointArm2_); // pro zoom
}

void AppManager::setAddPointMode(AddPointMode mode) {
    if (mode == currentAddPointMode_) return;
    currentAddPointMode_ = mode;
    qDebug() << "nastaven mode "  << modeAddPointToString(mode);
    if (currentAddPointMode_ == AddPointMode::Polyline)  //prvni bod do polyline
    {
        addPolylinetoShapeManager();
    }
    emit modeAddPointChanged(mode);
}

void AppManager::setContiMode(ContiMode mode) {
    qDebug() << "contimode " << modeContiToString(mode);
    currentContiMode_ = mode;
    emit modeContiChanged(mode);
}

AddPointMode AppManager::getAddPointMode() const {
    return currentAddPointMode_;
}

QString AppManager::modeAddPointToString(AddPointMode mode) {
    switch (mode) {
    case AddPointMode::None:      return "None";
    case AddPointMode::Calibrate: return "Calibration";
    case AddPointMode::Circle:    return "Circle";
    case AddPointMode::Polyline:  return "Polyline";
    case AddPointMode::Measure:   return "Measure";
    default:                      return "Unknown";
    }
}

QString AppManager::modeContiToString(ContiMode mode) {
    switch (mode) {
    case ContiMode::Continous:   return "Continous";
    case ContiMode::SinglePoint: return "Single Point";
    default:                     return "Unknown";
    }
}

void AppManager::cancelCurrentAction() {
    setAddPointMode(AddPointMode::None);
}

void AppManager::addpointfrommainwindow(void)
{
    qDebug() << "add point from main window position " << endPointArm2_;
    lastpoint = endPointArm2_;
    switch (currentAddPointMode_) {
        case AddPointMode::None:
            addPointtoShapeManager();
            break;
        case AddPointMode::Circle:
            // TODO: doplnit circle fit
            break;
        case AddPointMode::Polyline:
            qDebug() << "polyline add point";
            break;
        case AddPointMode::Measure:
            qDebug() << "case measure add point";
            addPointtoMeasuru();
            break;
        default:
            break;
    }
    emit sceneModified(endPointArm2_);
}

void AppManager::addPolylinetoShapeManager()
{
    // TODO: tvůj kód pro polyline (ponecháno zakomentované v původní verzi)
}

void AppManager::addPointtoShapeManager()
{
    lastpoint = endPointArm2_;
    mypoint* point = new mypoint();
    point->setPos(endPointArm2_);
    scene_->addItem(point);
}

void AppManager::addPointtoMeasuru()
{
    measure_->mode++;
    if (measure_->mode >= 3) measure_->mode = 1;

    if (measure_->mode == 0){
        measure_->set_color(Qt::black);
    }
    if (measure_->mode == 1)
    {
        measure_->set_color(Qt::red);
        measure_->start_position = endPointArm2_;
    }
    if (measure_->mode == 2) {
        measure_->set_color(Qt::green);
    }
    emit armsUpdated(Arm1Angle_, Arm2Angle_, endPointArm1_, endPointArm2_);
}

void AppManager::setScene(QGraphicsScene *scene)
{
    scene_ = scene;
}

void AppManager::setMeasure(MeasureDialog *measure)
{
    measure_ = measure;
}

void AppManager::setSettingsManager(SettingsManager* m)
{
    settingsManager_ = m;
    if (!settingsManager_) return;

    // Inicializuj cache ihned
    settings_ = settingsManager_->currentSettings();
    if (serialmanager_) serialmanager_->applySettings(settings_);
    connect(settingsManager_, &SettingsManager::settingsChanged,this, [this](const Settings& s){
                settings_ = s;
                if (serialmanager_) serialmanager_->applySettings(settings_); });

    // a drž v sync po změnách
    connect(settingsManager_, &SettingsManager::settingsChanged,
            this, [this](const Settings& s){ settings_ = s; });
}


/* =====================  NOVÉ: SerialManager napojení  ===================== */

void AppManager::setSerialManager(SerialManager* sm)
{
    // pokud už je nějaký napojený, odpoj ho (prevence duplicitních signálů)
    if (serialmanager_ && serialmanager_ != sm) {
        serialmanager_->disconnect(this);
    }

    serialmanager_ = sm;
    if (!serialmanager_) return;

    // --- signály SerialManageru -> AppManager/UI ---
    connect(serialmanager_, &SerialManager::opened,
            this, &AppManager::serialOpened, Qt::UniqueConnection);

    connect(serialmanager_, &SerialManager::opened,
            this, &AppManager::onSerialOpened, Qt::UniqueConnection);

    connect(serialmanager_, &SerialManager::closed,
            this, &AppManager::serialClosed, Qt::UniqueConnection);
    connect(serialmanager_, &SerialManager::closed,
            this, &AppManager::onSerialClosed, Qt::UniqueConnection);

    connect(serialmanager_, &SerialManager::errorOccured,
            this, &AppManager::serialError, Qt::UniqueConnection);

    // napojení přijatých rámců na slot, který je převádí na QByteArray
    connect(serialmanager_, &SerialManager::frameReceived,
            this, &AppManager::onSerialData,
            Qt::UniqueConnection);

    connect(this, &AppManager::serialData,
            this, &AppManager::onSerialLine,
            Qt::UniqueConnection);


    // korektní ukončení při zavření aplikace – stačí JEDNO připojení
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]{
        if (serialmanager_) serialmanager_->close();
    });
}

void AppManager::openSerial()
{
    if (!serialmanager_) return;

    if (settingsManager_) {
            settings_ = settingsManager_->currentSettings();    // nebo currentSettings()
            serialmanager_->applySettings(settings_);
        }
        QMetaObject::invokeMethod(serialmanager_, "open", Qt::QueuedConnection);
}

void AppManager::closeSerial()
{
    if (!serialmanager_) return;
    QMetaObject::invokeMethod(serialmanager_, "close", Qt::QueuedConnection);
   }


void AppManager::send(const QByteArray& data)
{
    if (!serialmanager_) return;
       QMetaObject::invokeMethod(serialmanager_, "send", Qt::QueuedConnection,
                                 Q_ARG(QByteArray, data));
}

<<<<<<< HEAD
=======
void AppManager::onSerialData(const Frame& frame)
{
    emit serialData(frame.data);
}

>>>>>>> pr-19
void AppManager::onSerialLine(const QByteArray& line)
{
    if (line.startsWith("#A:")) {
        alfa_ = line.mid(3).trimmed().toDouble();
<<<<<<< HEAD
    } else if (line.startsWith("#B:")) {
        beta_ = line.mid(3).trimmed().toDouble();
    } else if (line.startsWith("#I:")) {
        const int index = line.mid(3).trimmed().toInt();
        setAngles(alfa_, beta_, index);
    }
}


=======
        alfaReceived_ = true;
    } else if (line.startsWith("#B:")) {
        beta_ = line.mid(3).trimmed().toDouble();
        betaReceived_ = true;
    } else if (line.startsWith("#I:")) {
        const int index = line.mid(3).trimmed().toInt();
        if (alfaReceived_ && betaReceived_) {
            setAngles(alfa_, beta_, index);
            alfaReceived_ = betaReceived_ = false;
        }
    }
}

>>>>>>> pr-19
void AppManager::onSerialOpened()
{
    qDebug() << "AppManager: opened -" << dataSourceToHuman();
    // pošli čitelnou hlášku ven (MainWindow -> statusbar)
    emit connectionNotice(tr("Připojeno: %1").arg(dataSourceToHuman()));
}

void AppManager::onSerialClosed()
{
    qDebug() << "AppManager: close -" << dataSourceToHuman();
    emit connectionNotice(tr("Odpojeno: %1").arg(dataSourceToHuman()));
}

// pomocná funkce – název a detaily zdroje
QString AppManager::dataSourceToHuman() const
{
    switch (settings_.datasource) {
    case DataSource::Serial: {
        const QString port = settings_.serial.portName.isEmpty()
                             ? settings_.portName
                             : settings_.serial.portName;
        const int baud = settings_.serial.baudRate > 0
                         ? settings_.serial.baudRate
                         : settings_.baudRate;
        return tr("Sériový port %1 @ %2").arg(port).arg(baud);
    }
    case DataSource::Modbus: {
        if (settings_.modbus.isTcp) {
            return tr("Modbus/TCP %1:%2 (UnitId %3)")
                    .arg(settings_.modbus.host)
                    .arg(settings_.modbus.port)
                    .arg(settings_.modbus.serverId);
        } else {
            return tr("Modbus/RTU %1 @ %2 (UnitId %3)")
                    .arg(settings_.modbus.serialPort)
                    .arg(settings_.modbus.baudRate)
                    .arg(settings_.modbus.serverId);
        }
    }
    case DataSource::Simulation:
        return tr("Simulace: %1 (×%2)")
                .arg(settings_.simulation.logFile)
                .arg(settings_.simulation.speedFactor, 0, 'f', 2);
    case DataSource::None:
    default:
        return tr("Žádný zdroj");
    }
}



