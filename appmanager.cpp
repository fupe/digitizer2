#include "appmanager.h"
#include <QDebug>
#include <QCoreApplication>
#include <QMetaObject>
#include <cmath>
#include "serialmanager.h"
#include "settingsmanager.h"
#include "idatasourceworker.h"
#include "GraphicsItems.h"

namespace {
    constexpr double kPi = 3.14159265358979323846;
}

AppManager::AppManager(QObject* parent) : QObject(parent)
{
    connect(&shapeManager_, &ShapeManager::shapesChanged,
            this, &AppManager::onShapesChanged);
    dataSetTimer_.setInterval(10000);
    connect(&dataSetTimer_, &QTimer::timeout,
            this, &AppManager::logDataSetCount);
    dataSetTimer_.start();
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
    //qDebug() << "AppManager::setAngles input" << alfa << beta << index;
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
        emit positionChanged(endPointArm2_);
        //qDebug() << "AppManager::setAngles armsUpdated" << alfa_ << beta_ << endPointArm1_ << endPointArm2_;
    }

    indexBefore_ = index;

    // vzdálenost – použijeme robustní std::hypot
    distance = std::hypot(lastpoint.x() - endPointArm2_.x(),
                          lastpoint.y() - endPointArm2_.y());

    if (distance > settings_.auto_step && currentContiMode_ == ContiMode::Continous)
    {
        //qDebug() << "distance " << distance;
        addpointfrommainwindow();
    }
    else
    {
        emit sceneModified(endPointArm2_); // pro zoom
    }
}

void AppManager::setAddPointMode(AddPointMode mode) {
    if (mode == currentAddPointMode_) return;

    if (currentAddPointMode_ == AddPointMode::Polyline && mode != AddPointMode::Polyline) {
        shapeManager_.finishCurrent();
    }

    currentAddPointMode_ = mode;
    qDebug() << "nastaven mode "  << modeAddPointToString(mode);

    if (mode == AddPointMode::Polyline) {
        if (!shapeManager_.hasCurrent()) {
            auto *pl = new mypolyline(this);
            scene_->addItem(pl);
            shapeManager_.startShape(pl);
            shapeManager_.appendToCurrent(endPointArm2_);
            lastpoint = endPointArm2_;
        }
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

ContiMode AppManager::getContiMode() const {
    return currentContiMode_;
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
    //qDebug() << "add point from main window position " << endPointArm2_;
    lastpoint = endPointArm2_;
    switch (currentAddPointMode_) {
        case AddPointMode::None:
           // qDebug() << "add point" << modeAddPointToString(currentAddPointMode_);
            addPointtoShapeManager();
            break;
        case AddPointMode::Circle:
            // TODO: doplnit circle fit
            break;
        case AddPointMode::Polyline:
            //qDebug() << "polyline add point";
            if (shapeManager_.hasCurrent()) {
                shapeManager_.appendToCurrent(endPointArm2_);
            }
            break;
        case AddPointMode::Measure:
            //qDebug() << "case measure add point";
            addPointtoMeasuru();
            break;
        case AddPointMode::Calibrate:
            {
                double betaDeg = (beta_ - settings_.beta_offset) * 180.0 / kPi;
                emit calibrateAnglesAdded(Arm1Angle_, betaDeg);
            }
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

void AppManager::finishCurrentShape()
{
    shapeManager_.finishCurrent();
    setAddPointMode(AddPointMode::None);
}

void AppManager::clearShapeManager()
{
    shapeManager_.clear();
}

void AppManager::addPointtoShapeManager()
{
    // přebarvi předchozí poslední bod zpátky na červenou
    const auto& shapes = shapeManager_.getShapes();
    if (!shapes.isEmpty()) {
        if (auto prev = dynamic_cast<mypoint*>(shapes.last())) {
            prev->pen.setColor(Qt::red);
            prev->update();
        }
    }

    lastpoint = endPointArm2_;
    auto* point = new mypoint;
    point->setPos(endPointArm2_);
    point->pen.setColor(Qt::green); // nový poslední bod zeleně
    shapeManager_.addShape(point);
}

void AppManager::deleteLastPoint()
{
    const auto& shapes = shapeManager_.getShapes();
    if (shapes.isEmpty())
        return;

    // smaž pouze pokud je poslední položka bod
    if (!dynamic_cast<mypoint*>(shapes.last()))
        return;

    shapeManager_.deleteLastShape();

    const auto& updated = shapeManager_.getShapes();
    if (!updated.isEmpty()) {
        if (auto last = dynamic_cast<mypoint*>(updated.last())) {
            last->pen.setColor(Qt::green);
            last->update();
            lastpoint = last->pos();
        }
    } else {
        lastpoint = QPointF();
    }
}

void AppManager::addPointtoMeasuru()
{
    emit measureToggled(endPointArm2_);
}

void AppManager::onShapesChanged()
{
    if (!scene_) return;

    for (QGraphicsItem* item : scene_->items()) {
        if (dynamic_cast<GraphicsItems*>(item)) {
            scene_->removeItem(item);
        }
    }

    for (auto* shape : shapeManager_.getShapes()) {
        //qDebug()<<"AppManager::onShapesChanged :" <<shape;
        scene_->addItem(shape);
    }

    emit shapesChanged();
}

int AppManager::pointCount() const {
    int count = 0;
    for (auto* item : shapeManager_.getShapes()) {
        if (dynamic_cast<mypoint*>(item)) {
            ++count;
        }
    }
    return count;
}

int AppManager::polylineCount() const {
    int count = 0;
    for (auto* item : shapeManager_.getShapes()) {
        if (dynamic_cast<mypolyline*>(item)) {
            ++count;
        }
    }
    return count;
}

int AppManager::circleCount() const {
    int count = 0;
    for (auto* item : shapeManager_.getShapes()) {
        if (dynamic_cast<mycircle*>(item)) {
            ++count;
        }
    }
    return count;
}

void AppManager::setScene(QGraphicsScene *scene)
{
    scene_ = scene;
    onShapesChanged();
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
     }, Qt::UniqueConnection);
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


void AppManager::onSerialData(const Frame& frame)
{
    //qDebug() << "AppManager::onSerialData" << frame.data;
    emit serialData(frame.data);
}

void AppManager::onSerialLine(const QByteArray& line)
{
    //qDebug() << "AppManager::onSerialLine" << line;
    if (line.startsWith("#A:")) {
        alfa_ = line.mid(3).trimmed().toDouble();
        alfaReceived_ = true;
        //qDebug() << "alfa------------" << alfa_;
    } else if (line.startsWith("#B:")) {
        beta_ = line.mid(3).trimmed().toDouble();
        betaReceived_ = true;
    } else if (line.startsWith("#I:")) {
        const int index = line.mid(3).trimmed().toInt();
        if (alfaReceived_ && betaReceived_) {
            setAngles(alfa_, beta_, index);
            alfaReceived_ = betaReceived_ = false;
            ++dataSetCount_;
        }
    }
}

void AppManager::logDataSetCount()
{
   // qDebug() << "Přijaté sady dat:" << dataSetCount_;

}

void AppManager::onSerialOpened()
{
    serialConnected_ = true;
    qDebug() << "AppManager: opened -" << dataSourceToHuman();
    // pošli čitelnou hlášku ven (MainWindow -> statusbar)
    emit connectionNotice(tr("Připojeno: %1").arg(dataSourceToHuman()));
}

void AppManager::onSerialClosed()
{
    serialConnected_ = false;
    qDebug() << "AppManager: close -" << dataSourceToHuman();
    emit connectionNotice(tr("Odpojeno: %1").arg(dataSourceToHuman()));
    settings_.simulation.loggingEnabled = false;
    if (settingsManager_) settingsManager_->updateSettings(settings_);
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



