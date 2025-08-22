#pragma once
#include <QString>
#include <QKeySequence>
#include <QHash>
#include <QDebug>
#include <QSettings>
#include <QColorDialog>

// --- Units enum & helpers ---
enum class Units { Millimeters, Inches };

inline QString unitsToString(Units u) {
    return (u == Units::Millimeters) ? QStringLiteral("mm") : QStringLiteral("inch");
}
inline Units stringToUnits(const QString& s) {
    return (s.compare(QStringLiteral("inch"), Qt::CaseInsensitive) == 0)
        ? Units::Inches
        : Units::Millimeters;
}

//------------komunikace
enum class DataSource {
    Serial,     // klasický COM
    Modbus,     // Modbus RTU/TCP (do budoucna)
    Simulation, // přehrávání logu
    None        // odpojeno
};

// --- DataSource enum helpers ---
inline QString dataSourceToString(DataSource s) {
    switch (s) {
        case DataSource::Serial:     return QStringLiteral("serial");
        case DataSource::Modbus:     return QStringLiteral("modbus");
        case DataSource::Simulation: return QStringLiteral("simulation");
        case DataSource::None:       return QStringLiteral("none");
    }
    return QStringLiteral("none");
}
inline DataSource stringToDataSource(const QString& s) {
    const QString t = s.trimmed().toLower();
    if (t == QStringLiteral("serial"))     return DataSource::Serial;
    if (t == QStringLiteral("modbus"))     return DataSource::Modbus;
    if (t == QStringLiteral("simulation")) return DataSource::Simulation;
    return DataSource::None;
}


struct SerialParams {
    QString portName;
    int baudRate = 115200;
    int dataBits = 8;
    int stopBits = 1;
    int parity   = 0;   // 0=none,1=odd,2=even...
    int flow     = 0;   // 0=none,1=rtscts...
};

struct ModbusParams {
    bool isTcp = false;
    QString serialPort;   // pro RTU
    int baudRate = 115200;
    QString host;         // pro TCP
    int port = 502;       // pro TCP
    int serverId = 1;     // slave id
};

struct SimulationParams {
    QString logFile;        // cesta k logu s časovými značkami
    double speedFactor = 1; // 1.0 = realtime, >1 zrychlení
};


// --- Shortcuts container ---
struct Shortcuts {
    // Stable action IDs -> key sequences
    QHash<QString, QKeySequence> map;

    static Shortcuts defaults() ;
};

// --- Settings struct ---
struct Settings {
    // Serial
    QString portName = "COM1";
    int     baudRate = 115200;
    DataSource datasource = DataSource::Serial;  //prepinani komunikace
        SerialParams serial;
        ModbusParams modbus;
        SimulationParams simulation;
    // UI
    Units   units       = Units::Millimeters;
    QRect main_window_position;
    QRect measure_window_position;
    QPen *arms_pen;
    QColor arms_color;
    QString language;
    bool save_main_window_position_on_exit;
    bool save_measure_window_position_on_exit;


    //app
    double arm1_length;
    double arm2_length;
    double auto_step;
    double alfa_offset;
    double beta_offset;
    QString directory_save_dxf;
    QString directory_save_data;
    bool document_modified;
    bool document_saved;
    double units_scale;


    // Shortcuts
    Shortcuts shortcuts = Shortcuts::defaults();
};
