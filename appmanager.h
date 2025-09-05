#ifndef APPMANAGER_H
#define APPMANAGER_H
#include <QObject>
#include <QGraphicsScene>
#include <QPointF>
#include <QString>
#include <QByteArray>
#include "idatasourceworker.h"   // obsahuje struct Frame
#include "shapemanager.h"
#include "settings.h"          // kvůli struct Settings
#include "settingsmanager.h"
#include <QTimer>
#pragma once

class SerialManager; // forward deklarace

enum class AddPointMode {
     None,
     Point,
     Circle,
     Polyline,
     Measure,
     Calibrate
};

enum class ContiMode {
     SinglePoint,
     Continous
};

enum class ComMode {
    Disconnect,
    Conecting,
    Connect,
    Error
};

struct AppMode {
    AddPointMode addPointMode;
    ContiMode    contiMode;
    ComMode      comMode;

    AppMode(AddPointMode apm = AddPointMode::None,
            ContiMode cm = ContiMode::SinglePoint,
            ComMode com = ComMode::Disconnect)
        : addPointMode(apm), contiMode(cm), comMode(com) {}
};


class AppManager : public QObject {
    Q_OBJECT
public:
    explicit AppManager(QObject *parent = nullptr);
    void setSettingsManager(SettingsManager* m);
    void setSerialManager(SerialManager* sm);
    SettingsManager* settingsManager() const { return settingsManager_; }
    SerialManager* serialManager() const { return serialmanager_; }

    // --- Jednoduché API pro UI (otevření/zavření portu, zápis dat) ---
    void openSerial();                                // vezme port/baud ze SettingsManageru
    void closeSerial();
    void send(const QByteArray& data);
    QString dataSourceToHuman() const;           // pomocná human-readable hláška
    bool isSerialConnected() const { return serialConnected_; }

    void setAngles(double alfa, double beta,int index);
    double getAlfa();
    double getBeta();
    void setAddPointMode(AddPointMode mode);
    void setContiMode(ContiMode mode);
    AddPointMode getAddPointMode() const;
    ContiMode getContiMode() const;
    QString modeAddPointToString(AddPointMode mode);
    QString modeContiToString(ContiMode mode);
    void cancelCurrentAction();
    void addpointfrommainwindow(void);
    void setScene(QGraphicsScene* scene);
    void addPointtoMeasuru(void);
    void deleteLastPoint();
    void finishCurrentShape();
    void clearShapeManager();

    double distance;
    QPointF lastpoint;

    QPointF arm2EndPoint() const { return endPointArm2_; }
    GraphicsItems* currentShape() const { return shapeManager_.currentShape(); }
    int pointCount() const;
    int polylineCount() const;
    int circleCount() const;
    QVector<int> polylinePointCounts() const;

signals:
    // stávající signály
    void armsUpdated(double arm1Angle, double arm2Angle, QPointF endPointArm1_, QPointF endPointArm2_ );
    void positionChanged(QPointF pos);
    void measureToggled(QPointF pos);
    void modeAddPointChanged (AddPointMode newmode);
    void modeContiChanged(ContiMode mode);
    void sceneModified(QPointF endarm2);
    void calibrateAnglesAdded(double alfaDeg, double betaDeg);
    void shapesChanged();

    // --- Nové signály související se sériovým portem ---
    void serialOpened();                         // bez argumentu (matchuje SerialManager::opened())
    void serialClosed();
    void serialError(const QString& msg);
    void serialData(const QByteArray& data);     // ponecháme pro kompatibilitu s UI
    void connectionNotice(const QString& msg);   // text do statusbaru


private:
    ShapeManager    shapeManager_;
    Settings        settings_;                          // cache aktuálních settings
    SettingsManager* settingsManager_ = nullptr;        // ukazatel na settings manager
    SerialManager*   serialmanager_         = nullptr;         // ← ukazatel na SerialManager

    double Arm1Angle_{};
    double Arm2Angle_{};
    double alfa_{}, beta_{};
    bool   alfaReceived_ = false;
    bool   betaReceived_ = false;
    double x_value_{}, y_value_{};      // koncová pozice prvního ramene
    double x_value2_{}, y_value2_{};    // koncová pozice druhého ramene
    QPointF endPointArm1_;
    QPointF endPointArm2_;
    QPointF endPointBefore_;
    int     indexBefore_{};
    double  distance_{};
    AddPointMode currentAddPointMode_ = AddPointMode::None;
    ContiMode    currentContiMode_    = ContiMode::SinglePoint;
    QGraphicsScene* scene_ = nullptr;
    QTimer dataSetTimer_;
    int dataSetCount_ = 0;
    void logDataSetCount();
    bool serialConnected_ = false;

public slots:
    // zatím prázdné; ponecháno pro případné budoucí sloty
    void onSerialData(const Frame& frame);
    void onSerialOpened();
    void onSerialClosed();
    void onSerialLine(const QByteArray& line);
    void onShapesChanged();
};

#endif // APPMANAGER_H
